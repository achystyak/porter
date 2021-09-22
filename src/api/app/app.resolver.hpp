#pragma once

#include "../../base/resolver/resolver.hpp"
#include "../../base/auth/auth.guard.hpp"
#include "../../base/libs/json/json.hpp"

#include "../../base/curl/curl.hpp"

#include "../../env.hpp"
#include "app.service.hpp"
#include "../../base/auth/ability/ability.hpp"

using namespace nlohmann;

class AppResolver : public Resolver
{
public:
    AppResolver() : Resolver("/app")
    {
        resolve(
            GET, "/external", [=](const Request &request)
            {
                auto response = Curl::Get("https://www.google.com");
                json body = {
                    {"status", response.body.size()},
                    {"code", response.code}};

                return Response::json(body.dump());
            },
            RO_AUTH_GUARDED, {Action::Read, Domain::App});

        resolve(
            GET, "/config", [=](const Request &request)
            { return Response::json(env.dump()); });

        m_pub = publisher("/websocket");

        schedule("* * * * *", [=]()
                 { m_pub->publish("Hi from cron!"); });
    }

private:
    Publisher *m_pub;
};