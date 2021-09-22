#pragma once
#include <string>
#include <vector>
#include "../common/http.hpp"
#include "../publisher/publisher.hpp"
#include "../scheduler/scheduler.hpp"
#include "../auth/ability/ability.hpp"

class Server;

struct ResolveHandler
{
    HttpMethod method;
    std::string path;
    ResponseCallback callback;
    ResolverOptions options;
    Ability ability;
};

class Resolver
{
public:
    Resolver(const std::string &path);
    virtual ~Resolver();

    void resolve(
        HttpMethod method,
        const std::string &path,
        const ResponseCallback &callback,
        ResolverOptions options = RO_NONE,
        Ability ability = Ability());

    Scheduler *schedule(
        const std::string &cron,
        const ScheduleCallback &callback);

    void unschedule(Scheduler *scheduler);

    Publisher *publisher(
        const std::string &path,
        ResolverOptions options = RO_NONE);

    const std::string &path() const;
    const std::vector<ResolveHandler *> &handlers() const;
    const std::vector<Publisher *> &publishers() const;
    const std::vector<Scheduler *> &schedulers() const;

    Server *server() const;
    void setServer(Server *server);

    // Lifecycle
    virtual void serverDidLoad();
    virtual void serverWillDestroy();

private:
    Resolver(const Resolver &h) {}
    std::string m_path;
    Server *m_server;
    std::vector<ResolveHandler *> m_handlers;
    std::vector<Publisher *> m_publishers;
    std::vector<Scheduler *> m_schedulers;
};