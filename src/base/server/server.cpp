#include "server.hpp"
#include <exception>

#include "../libs/mongoose/mongoose.hpp"
#include "../resolver/resolver.hpp"
#include "../auth/auth.guard.hpp"
#include "../common/utils.hpp"
#include "../logger/logger.hpp"

Request getRequest(const mg_http_message *hm)
{
    std::map<std::string, std::string> headers;
    for (unsigned int i = 0; i < MG_MAX_HTTP_HEADERS; ++i)
    {
        const mg_http_header &header = hm->headers[i];
        if (header.name.len && header.value.len)
        {
            std::string name = std::string(header.name.ptr);
            std::string value = name;
            auto pos = name.find(":");
            if (pos != std::string::npos)
            {
                name = name.substr(0, pos);
                value = value.substr(pos + 2);

                auto valPos = value.find("\r\n");
                if (valPos != std::string::npos)
                {
                    value = value.substr(0, valPos);
                }
            }
            headers[name] = value;
        }
    }

    std::string uri = hm->uri.ptr ? Utils::formatted(std::string(hm->uri.ptr), " HTTP") : "";
    std::string method = hm->method.ptr ? Utils::formatted(std::string(hm->method.ptr), " /") : "";
    std::string body = hm->body.ptr ? std::string(hm->body.ptr) : "";
    auto query = hm->query.ptr
                     ? Utils::params(std::string(hm->query.ptr))
                     : std::map<std::string, std::string>();

    return Request(Request::parseMethod(method), uri, body, headers, query);
}

// Init
// ===========================================================================================

Server::Server()
    : m_authGuard(nullptr),
      m_http_manager(nullptr),
      m_abort(false),
      m_port(0)
{
}

Server::~Server()
{
    stop();
    release();
}

// Lifecycle Logic
// ===========================================================================================

void Server::start(const std::string &host, const size_t &port)
{
    m_host = host;
    m_port = port;

    std::string http_address = "http://" + host + ":" + std::to_string(port);

    Log_Success("Server listening on %s (port %d)", http_address.c_str(), m_port);

    m_http_manager = new mg_mgr();

    mg_mgr_init(m_http_manager);

    mg_http_listen(
        m_http_manager, http_address.c_str(), [=](struct mg_connection *c, int ev, void *ev_data, void *fn_data)
        { handleHttp(c, ev, ev_data, fn_data); },
        NULL);

    while (!m_abort)
    {
        mg_mgr_poll(m_http_manager, 1000);
    }
    release();
}

void Server::stop()
{
    m_abort = true;
}

void Server::release()
{
    m_callbacks.clear();

    if (m_http_manager)
    {
        mg_mgr_free(m_http_manager);
        delete m_http_manager;
        m_http_manager = nullptr;
    }

    for (auto resolver : m_resolvers)
    {
        if (resolver.second)
        {
            resolver.second->setServer(nullptr);
            delete resolver.second;
            resolver.second = nullptr;
        }
    }
    m_resolvers.clear();

    std::map<Server *, ctpl::thread_pool *> buffer = m_children;
    for (auto bf : buffer)
    {
        if (bf.first)
        {
            removeChildServer(bf.first);
        }
    }
    buffer.clear();
    m_children.clear();

    std::vector<Queuer *> qbuffer = m_queuers;
    for (auto bf : qbuffer)
    {
        removeQueuer(bf);
    }
    qbuffer.clear();
    m_queuers.clear();

    delete m_authGuard;
    m_authGuard = nullptr;
}

// Http Logic
// ===========================================================================================

void Server::handleHttp(mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_HTTP_MSG)
    {
        try
        {
            struct mg_http_message *hm = (mg_http_message *)ev_data;
            Request request = getRequest(hm);

            if (!handleHttpCallbacks(c, request, hm) &&
                !handleHttpResolvers(c, request, hm) &&
                !handleHttpPublishers(c, request, hm))
            {
                auto response = Response::notFound();
                mg_http_reply(c, response.code, response.headers.c_str(), response.body.c_str());
            }
        }
        catch (const std::exception &exc)
        {
            Log_Error("%s", exc.what());
            std::string what = exc.what();
            try
            {
                if (json::accept(what))
                {
                    json error = json::parse(what);
                    if (error.contains("error") && error["error"].contains("code"))
                    {
                        Response response = Response::json(what);
                        response.code = error["error"]["code"];
                        mg_http_reply(c, response.code, response.headers.c_str(), response.body.c_str());
                        return;
                    }
                }
            }
            catch (...)
            {
            }
            auto response = Response::internalError();
            mg_http_reply(c, response.code, response.headers.c_str(), response.body.c_str());
        }
    }
    else if (ev == MG_EV_WS_MSG)
    {
        struct mg_ws_message *wm = (struct mg_ws_message *)ev_data;
        mg_ws_send(c, wm->data.ptr, wm->data.len, WEBSOCKET_OP_TEXT);
        mg_iobuf_del(&c->recv, 0, c->recv.len);
    }
    (void)fn_data;
}

bool Server::handleHttpCallbacks(mg_connection *c, Request &request, struct mg_http_message *hm)
{
    for (auto it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
    {
        if (mg_http_match_uri(hm, it->first.c_str()))
        {
            Response response(it->second(request));
            mg_http_reply(c, response.code, response.headers.c_str(), response.body.c_str());
            return true;
        }
    }
    return false;
}

bool Server::handleHttpResolvers(mg_connection *c, Request &request, struct mg_http_message *hm)
{
    for (auto it = m_resolvers.begin(); it != m_resolvers.end(); ++it)
    {
        std::string resolverPath = it->first;
        const auto &handlers = it->second->handlers();

        for (ResolveHandler *handler : handlers)
        {
            std::string path = resolverPath + handler->path;
            std::string method = hm->method.ptr ? Utils::formatted(std::string(hm->method.ptr), " /") : "";

            if (mg_http_match_uri(hm, path.c_str()) && request.method == handler->method)
            {
                if ((handler->options & RO_AUTH_GUARDED) == RO_AUTH_GUARDED)
                {
                    if (m_authGuard->authenticate(request) == false)
                    {
                        auto response = Response::unauthorized();
                        mg_http_reply(c, response.code, response.headers.c_str(), response.body.c_str());
                        return true;
                    }
                    if (AbilityFactory::shared()->verify(request.session["_id"], handler->ability) == false)
                    {
                        auto response = Response::forbidden();
                        mg_http_reply(c, response.code, response.headers.c_str(), response.body.c_str());
                        return true;
                    }
                }

                Response response(handler->callback(request));
                mg_http_reply(c, response.code, response.headers.c_str(), response.body.c_str());
                return true;
            }
        }
    }
    return false;
}

bool Server::handleHttpPublishers(mg_connection *c, Request &request, struct mg_http_message *hm)
{
    for (auto it = m_resolvers.begin(); it != m_resolvers.end(); ++it)
    {
        std::string resolverPath = it->first;
        const auto &publishers = it->second->publishers();

        for (Publisher *publisher : publishers)
        {
            std::string path = resolverPath + publisher->path();
            if (mg_http_match_uri(hm, path.c_str()))
            {
                if ((publisher->options() & RO_AUTH_GUARDED) != RO_AUTH_GUARDED ||
                    m_authGuard->authenticate(request, true))
                {
                    publisher->pushConnection(std::to_string(c->id), request.session);
                    publisher->subscribe(
                        [=](
                            const std::map<std::string, nlohmann::json> &cn,
                            const std::string &msg)
                        {
                            std::vector<std::string> ids;
                            for (auto &k : cn)
                            {
                                ids.push_back(k.first);
                            }
                            publish(ids, msg);
                        });
                    mg_ws_upgrade(c, hm, NULL);
                    return true;
                }
                return false;
            }
        }
    }
    return false;
}

// Auth Logic
// ===========================================================================================

void Server::useAuthGuard(AuthGuard *guard)
{
    m_authGuard = guard;
}

AuthGuard *Server::authGuard() const
{
    return m_authGuard;
}

// WebSockets Logic
// ===========================================================================================

void Server::publish(std::vector<std::string> ids, const std::string &message)
{
    try
    {
        for (mg_connection *c = m_http_manager->conns; c != NULL; c = c->next)
        {
            if (std::find(ids.begin(), ids.end(), std::to_string(c->id)) != ids.end())
            {
                mg_ws_send(c, message.c_str(), message.size(), WEBSOCKET_OP_TEXT);
            }
        }
    }
    catch (const std::exception &exc)
    {
        Log_Error("%s", exc.what());
    }
}

// Resolvers Logic
// ===========================================================================================

void Server::resolve(const std::string &path, const ResponseCallback &callback)
{
    m_callbacks[path] = callback;
}

void Server::resolve(Resolver *resolver)
{
    m_resolvers[resolver->path()] = resolver;
    resolver->setServer(this);
}

// Child Servers Logic
// ===========================================================================================

std::vector<Server *> Server::children() const
{
    std::vector<Server *> children;
    for (auto ms : m_children)
    {
        children.push_back(ms.first);
    }
    return children;
}

Server *Server::createChildServer(const std::string &host, const size_t &port)
{
    Server *child = new Server();
    auto pool = new ctpl::thread_pool(1);
    pool->push([child, host, port](int id)
               { child->start(host, port); });

    m_children[child] = pool;
    return child;
}

void Server::removeChildServer(Server *child)
{
    auto it = m_children.find(child);
    if (it != m_children.end())
    {
        it->first->stop();
        it->second->stop();

        delete (it->second);
        delete (it->first);

        m_children.erase(it);
    }
}

// Queuers Logic
// ===========================================================================================

Queuer *Server::addQueuer(Queuer *queuer)
{
    m_queuers.push_back(queuer);
    return queuer;
}

void Server::removeQueuer(Queuer *queuer)
{
    auto pos = std::find(m_queuers.begin(), m_queuers.end(), queuer);
    if (pos != m_queuers.end())
    {
        (*pos)->disconnect();
        delete (*pos);
        m_queuers.erase(pos);
    }
}

std::vector<Queuer *> Server::hostQueuers()
{
    std::vector<Queuer *> queuers;
    for (auto queuer : m_queuers)
    {
        if (queuer->isHost() == true)
        {
            queuers.push_back(queuer);
        }
    }
    return queuers;
}

std::vector<Queuer *> Server::clientQueuers()
{
    std::vector<Queuer *> queuers;
    for (auto queuer : m_queuers)
    {
        if (queuer->isHost() == false)
        {
            queuers.push_back(queuer);
        }
    }
    return queuers;
}
