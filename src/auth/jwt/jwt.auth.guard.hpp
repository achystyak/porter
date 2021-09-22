#pragma once
#include <exception>

#include "../../base/logger/logger.hpp"
#include "../../base/auth/auth.guard.hpp"
#include "../../env.hpp"
#include "../../base/libs/jwt-cpp/jwt.h"

#include "../ability/ability.guard.hpp"

using namespace nlohmann;

// JWT Auth Guard
// ======================================================================================================

class JwtAuthGuard : public AuthGuard
{
public:
    JwtAuthGuard()
        : AuthGuard(),
          m_abilityGuard(new DefaultAbilityGuard()) {}

    virtual ~JwtAuthGuard()
    {
        delete m_abilityGuard;
        m_abilityGuard = nullptr;
    }

    std::string encode(const nlohmann::json &payload) override
    {
        auto time = std::chrono::system_clock::now();
        picojson::value claim;
        picojson::parse(claim, payload.dump());

        auto token = jwt::create()
                         .set_issuer("auth0")
                         .set_issued_at(time)
                         .set_expires_at(time + std::chrono::seconds{env["jwt"]["timeout"]})
                         .set_payload_claim("user", claim)
                         .sign(jwt::algorithm::hs256{env["jwt"]["secret"]});
        return token;
    }

    nlohmann::json decode(const std::string &jwt) override
    {
        nlohmann::json result;
        try
        {
            auto decoded = jwt::decode(jwt);
            auto payload = decoded.get_payload_claim("user");
            result = nlohmann::json::parse(payload.to_json().serialize());
        }
        catch (const std::exception &exc)
        {
            Log_Error("Wrong JWT: %s", exc.what());
        }
        return result;
    }

    bool validate(const std::string &jwt) override
    {
        try
        {
            auto decoded = jwt::decode(jwt);
            auto verifier = jwt::verify()
                                .allow_algorithm(jwt::algorithm::hs256{env["jwt"]["secret"]})
                                .with_issuer("auth0");
            verifier.verify(decoded);
            return true;
        }
        catch (const std::exception &exc)
        {
            Log_Error("%s", exc.what());
        }
        return false;
    }

    bool authenticate(Request &request, bool wss) override
    {
        try
        {
            const auto &headers = wss ? request.queries : request.headers;
            std::string key = wss ? "token" : "Authorization";
            if (headers.find(key) != headers.end())
            {
                std::string jwt = headers.at(key);
                if (validate(jwt))
                {
                    request.session = decode(jwt);
                    if (request.session.contains("_id") && request.session.at("_id").is_object())
                    {
                        m_abilityGuard->buildAbilities(AbilityFactory::shared(), request.session);
                        Log_Success("Auth granted: %s", jwt.c_str());
                        return true;
                    }
                }
            }
            return false;
        }
        catch (const std::exception &exc)
        {
            Log_Error("%s", exc.what());
            return false;
        }
    }

private:
    AbilityGuard *m_abilityGuard;
};