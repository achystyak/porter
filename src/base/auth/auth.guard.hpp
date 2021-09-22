#pragma once

#include "../libs/json/json.hpp"
#include "../common/http.hpp"

using namespace nlohmann;

class AuthGuard
{
public:
    AuthGuard() {}
    virtual ~AuthGuard() {}

    virtual std::string encode(const nlohmann::json &payload) = 0;
    virtual nlohmann::json decode(const std::string &jwt) = 0;
    virtual bool validate(const std::string &jwt) = 0;
    virtual bool authenticate(Request &request, bool wss = false) = 0;
};