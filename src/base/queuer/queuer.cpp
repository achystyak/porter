#include "queuer.hpp"

#include "../libs/mongoose/mongoose.hpp"
#include "../logger/logger.hpp"

struct sub
{
    struct sub *next;
    struct mg_connection *c;
    struct mg_str topic;
    uint8_t qos;
};
static struct sub *s_subs = NULL;

Queuer::Queuer()
    : m_connection(nullptr),
      m_done(false)
{
}

Queuer::~Queuer()
{
}

Queuer *Queuer::client(const std::string &host, const size_t &port)
{
    Queuer *queuer = new Queuer();
    queuer->m_isServer = false;
    queuer->m_pool = new ctpl::thread_pool(1);
    queuer->m_pool->push(
        [queuer, host, port](int id)
        {
            std::string address = "mqtt://" + host + ":" + std::to_string(port);
            mg_mgr *manager = new mg_mgr();
            mg_mgr_init(manager);

            struct mg_mqtt_opts opts;
            memset(&opts, 0, sizeof(opts));
            opts.qos = 1;

            mg_mqtt_connect(
                manager, address.c_str(), &opts,
                [=](mg_connection *c, int ev, void *ev_data, void *fn_data)
                { queuer->handleClient(c, ev, ev_data, fn_data); },
                &queuer->m_done);

            while (queuer->m_done == false)
            {
                mg_mgr_poll(manager, 1000);
            }
            mg_mgr_free(manager);
            delete manager;
            manager = nullptr;
        });
    return queuer;
}

Queuer *Queuer::host(const std::string &host, const size_t &port)
{
    Queuer *queuer = new Queuer();
    queuer->m_isServer = true;
    queuer->m_pool = new ctpl::thread_pool(1);
    queuer->m_pool->push(
        [queuer, host, port](int id)
        {
            std::string address = "mqtt://" + host + ":" + std::to_string(port);
            mg_mgr *manager = new mg_mgr();
            mg_mgr_init(manager);
            mg_mqtt_listen(
                manager, address.c_str(), [=](struct mg_connection *c, int ev, void *ev_data, void *fn_data)
                { queuer->handleServer(c, ev, ev_data, fn_data); },
                NULL);
            while (queuer->m_done == false)
            {
                mg_mgr_poll(manager, 1000);
            }
        });

    return queuer;
}

void Queuer::disconnect()
{
    m_done = true;
    m_pool->stop(true);
    delete m_pool;
    m_pool = nullptr;
}

bool Queuer::send(const std::string &topic, const std::string &message)
{
    mg_str _topic = mg_str_s(topic.c_str());
    mg_str _data = mg_str_s(message.c_str());

    if (m_isServer)
    {
        for (sub *_sub = s_subs; _sub != NULL; _sub = _sub->next)
        {
            if (mg_strcmp(mg_str_s(topic.c_str()), _sub->topic) != 0)
                continue;

            mg_mqtt_pub(_sub->c, &_topic, &_data, 1, false);
            return true;
        }
    }
    else
    {
        if (m_connection && !m_done)
        {
            mg_mqtt_sub(m_connection, &_topic, 1);
            mg_mqtt_pub(m_connection, &_topic, &_data, 1, false);
            return true;
        }
    }
    return false;
}

void Queuer::subscribe(const QueuerCallback &callback)
{
    m_callback = callback;
}

bool Queuer::isHost() const
{
    return m_isServer;
}

/// Queue Client Logic
// ===========================================================================================

void Queuer::handleClient(mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_MQTT_OPEN)
    {
        m_connection = c;
    }
    else if (ev == MG_EV_MQTT_MSG)
    {
        struct mg_mqtt_message *mm = (struct mg_mqtt_message *)ev_data;
        Log_Info("Recieved: %s", mm->data.ptr);

        if (m_callback && mm && mm->data.ptr && mm->topic.ptr)
        {
            m_callback(std::string(mm->topic.ptr), std::string(mm->data.ptr));
        }
    }
    else if (ev == MG_EV_ERROR || ev == MG_EV_CLOSE)
    {
        *(bool *)fn_data = true;
    }
}

/// Queue Server Logic
// ===========================================================================================

void Queuer::handleServer(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_MQTT_CMD)
    {
        struct mg_mqtt_message *mm = (struct mg_mqtt_message *)ev_data;
        switch (mm->cmd)
        {
        case MQTT_CMD_CONNECT:
        {
            uint8_t response[] = {0, 0};
            mg_mqtt_send_header(c, MQTT_CMD_CONNACK, 0, sizeof(response));
            mg_send(c, response, sizeof(response));
            break;
        }
        case MQTT_CMD_SUBSCRIBE:
        {
            int pos = 4;
            uint8_t qos;
            struct mg_str topic;
            while ((pos = mg_mqtt_next_sub(mm, &topic, &qos, pos)) > 0)
            {
                sub *_sub = new sub();
                _sub->c = c;
                _sub->topic = mg_strdup(topic);
                _sub->qos = qos;
                LIST_ADD_HEAD(sub, &s_subs, _sub);
            }
            break;
        }
        case MQTT_CMD_PUBLISH:
        {
            if (mm && mm->topic.ptr && mm->data.ptr)
            {
                std::string topic = std::string(mm->topic.ptr);
                std::string message = std::string(mm->data.ptr);
                Log_Info("Published: %s", mm->data.ptr);

                send(topic, message);

                if (m_callback)
                {
                    m_callback(topic, message);
                }
            }
            break;
        }
        }
    }
    else if (ev == MG_EV_CLOSE)
    {
        for (sub *next, *_sub = s_subs; _sub != NULL; _sub = next)
        {
            next = _sub->next;
            if (c != _sub->c)
                continue;
            LIST_DELETE(sub, &s_subs, _sub);
        }
    }
}
