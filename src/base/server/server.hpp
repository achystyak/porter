#pragma once

#include <string>
#include <map>

#include "../common/http.hpp"
#include "../queuer/queuer.hpp"
#include "../libs/scheduler/ctpl_stl.h"
#include "../auth/ability/ability.factory.hpp"

struct mg_connection;
struct mg_mgr;

class Resolver;
class AuthGuard;
class Queuer;

class Server
{
public:
    Server();
    ~Server();

    void start(const std::string &host, const size_t &port);
    void stop();

    void resolve(const std::string &path, const ResponseCallback &callback);
    void resolve(Resolver *resolver);

    AuthGuard *authGuard() const;
    void useAuthGuard(AuthGuard *guard);

    std::vector<Server *> children() const;
    Server *createChildServer(const std::string &host, const size_t &port);
    void removeChildServer(Server *child);

    std::vector<Queuer *> hostQueuers();
    std::vector<Queuer *> clientQueuers();

    Queuer *addQueuer(Queuer *queuer);
    void removeQueuer(Queuer *queuer);

private:
    Server(const Server &h) {}

    void publish(std::vector<std::string> ids, const std::string &message);

    void handleHttp(mg_connection *c, int ev, void *ev_data, void *fn_data);
    bool handleHttpCallbacks(mg_connection *c, Request &request, struct mg_http_message *hm);
    bool handleHttpResolvers(mg_connection *c, Request &request, struct mg_http_message *hm);
    bool handleHttpPublishers(mg_connection *c, Request &request, struct mg_http_message *hm);

    void release();

    std::string m_host;
    size_t m_port;
    bool m_abort;
    mg_mgr *m_http_manager;
    AuthGuard *m_authGuard;

    std::map<std::string, ResponseCallback> m_callbacks;
    std::map<std::string, Resolver *> m_resolvers;
    std::map<Server *, ctpl::thread_pool *> m_children;
    std::vector<Queuer *> m_queuers;
};