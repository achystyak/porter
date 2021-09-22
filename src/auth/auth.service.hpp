#pragma once
#include <exception>

#include "../base/service/service.hpp"
#include "../base/libs/json/json.hpp"
#include "../base/common/shared.hpp"
#include "../base/logger/logger.hpp"
#include "../api/user/user.hpp"

using namespace nlohmann;

class AuthService : public Service<DB_User::User>
{
    SERVICE(AuthService, Domain::Auth, "users")

    json signup(const Request &request)
    {
        json result;
        try
        {
            if (json::accept(request.body))
            {
                json payload = json::parse(request.body);
                if (payload.contains("email") && payload["email"].is_string() &&
                    payload.contains("password") && payload["password"].is_string())
                {
                    result = {{"email", payload["email"]},
                              {"password", payload["password"]}};

                    this->create(result);
                }
            }
        }
        catch (const std::exception &exc)
        {
            Log_Error("%s", exc.what());
        }
        return result;
    }

    json login(const Request &request)
    {
        json result;
        try
        {
            if (json::accept(request.body))
            {
                json payload = json::parse(request.body);
                if (payload.contains("email") && payload["email"].is_string() &&
                    payload.contains("password") && payload["password"].is_string())
                {
                    json filter = {{"email", payload["email"]},
                                   {"password", payload["password"]}};

                    auto data = this->find(filter);
                    if (data.size())
                    {
                        result = {{"_id", data.front().id}};
                    }
                }
            }
        }
        catch (const std::exception &exc)
        {
            Log_Error("%s", exc.what());
        }

        return result;
    }
};