#pragma once
#include "../base/resolver/resolver.hpp"
#include "../base/auth/auth.guard.hpp"
#include "../base/libs/json/json.hpp"
#include "../base/server/server.hpp"

#include "auth.service.hpp"

using namespace nlohmann;

class AuthResolver : public Resolver
{
public:
    AuthResolver() : Resolver("/auth")
    {
        resolve(
            POST, "/signup", [=](const Request &request)
            { return this->signup(request); });

        resolve(
            POST, "/login", [=](const Request &request)
            { return this->login(request); });

        resolve(
            GET, "/session", [=](const Request &request)
            { return Response::json(request.session.dump()); },
            RO_AUTH_GUARDED);
    }

    Response signup(const Request &request)
    {
        json user = AuthService::shared()->signup(request);
        if (user.is_object())
        {
            return Response::json(user.dump());
        }
        return Response::badRequest();
    }

    Response login(const Request &request)
    {
        json user = AuthService::shared()->login(request);
        if (user.is_object() && server() && server()->authGuard())
        {
            auto jwt = server()->authGuard()->encode(user);
            json response = {{"token", jwt}};

            return Response::json(response.dump());
        }
        return Response::badRequest();
    }
};