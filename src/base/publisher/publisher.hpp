#pragma once

#include "../common/http.hpp"

class Publisher
{
public:
    Publisher(const std::string &path, ResolverOptions options = RO_NONE)
        : m_path(path),
          m_options(options)
    {
    }

    const std::string &path() const
    {
        return m_path;
    }

    const ResolverOptions options() const
    {
        return m_options;
    }

    const std::map<std::string, nlohmann::json> &connections() const
    {
        return m_connections;
    }

    void subscribe(const PublishCallback &callback)
    {
        m_callback = callback;
    }

    void publish(const std::string &message)
    {
        if (m_callback)
        {
            m_callback(m_connections, message);
        }
    }

    void pushConnection(const std::string &connection, const nlohmann::json &session)
    {
        m_connections[connection] = session;
    }

private:
    std::string m_path;
    std::map<std::string, nlohmann::json> m_connections;
    PublishCallback m_callback;
    ResolverOptions m_options;
};