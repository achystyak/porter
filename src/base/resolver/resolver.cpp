#include "resolver.hpp"

Resolver::Resolver(const std::string &path)
    : m_path(path)
{
}

Resolver::~Resolver()
{
    for (auto handler : m_handlers)
    {
        delete handler;
        handler = nullptr;
    }
    m_handlers.clear();

    for (auto publisher : m_publishers)
    {
        delete publisher;
        publisher = nullptr;
    }
    m_publishers.clear();

    for (auto sheduler : m_schedulers)
    {
        delete sheduler;
        sheduler = nullptr;
    }
    m_schedulers.clear();
}

const std::string &Resolver::path() const
{
    return m_path;
}

const std::vector<ResolveHandler *> &Resolver::handlers() const
{
    return m_handlers;
}

const std::vector<Publisher *> &Resolver::publishers() const
{
    return m_publishers;
}

const std::vector<Scheduler *> &Resolver::schedulers() const
{
    return m_schedulers;
}

Server *Resolver::server() const
{
    return m_server;
}

void Resolver::setServer(Server *server)
{
    if (server)
    {
        m_server = server;
        serverDidLoad();
    }
    else
    {
        serverWillDestroy();
        m_server = server;
    }
}

void Resolver::serverDidLoad()
{
}

void Resolver::serverWillDestroy()
{
}

void Resolver::resolve(
    HttpMethod method,
    const std::string &path,
    const ResponseCallback &callback,
    ResolverOptions options,
    Ability ability)
{
    ResolveHandler *handler = new ResolveHandler();
    handler->method = method;
    handler->path = path;
    handler->options = options;
    handler->callback = callback;
    handler->ability = ability;
    m_handlers.push_back(handler);
}

Scheduler *Resolver::schedule(
    const std::string &cron,
    const ScheduleCallback &callback)
{
    Scheduler *scheduler = new Scheduler(cron, callback);
    m_schedulers.push_back(scheduler);
    return scheduler;
}

void Resolver::unschedule(Scheduler *scheduler)
{
    auto pos = std::find(m_schedulers.begin(), m_schedulers.end(), scheduler);
    if (pos != m_schedulers.end())
    {
        delete *pos;
        m_schedulers.erase(pos);
    }
}

Publisher *Resolver::publisher(
    const std::string &path,
    ResolverOptions options)
{
    for (auto sub : m_publishers)
        if (sub->path() == path)
            return sub;

    Publisher *publisher = new Publisher(path, options);
    m_publishers.push_back(publisher);
    return publisher;
}