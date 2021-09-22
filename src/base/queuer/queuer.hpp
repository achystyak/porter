#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>

#include "../common/http.hpp"
#include "../libs/scheduler/ctpl_stl.h"

struct Mongoose;

typedef std::function<void(const std::string &, const std::string &)> QueuerCallback;

class Queuer
{
public:
    ~Queuer();

    static Queuer *host(const std::string &host, const size_t &port);
    static Queuer *client(const std::string &host, const size_t &port);

    void disconnect();

    bool send(const std::string &topic, const std::string &message);
    void subscribe(const QueuerCallback &callback);

    bool isHost() const;

private:
    Queuer();
    Queuer(const Queuer &q) = delete;
    Queuer &operator=(const Queuer &q) = delete;

    void handleServer(struct mg_connection *c, int ev, void *ev_data, void *fn_data);
    void handleClient(struct mg_connection *c, int ev, void *ev_data, void *fn_data);

    mg_connection *m_connection;
    QueuerCallback m_callback;
    ctpl::thread_pool *m_pool;
    bool m_done;
    bool m_isServer;
};