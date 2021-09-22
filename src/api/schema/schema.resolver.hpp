#pragma once
#include "../../base/resolver/resolver.hpp"
#include "../../base/auth/auth.guard.hpp"
#include "../../base/libs/json/json.hpp"

#include "schema.service.hpp"

using namespace nlohmann;

class SchemaResolver : public Resolver
{
public:
    SchemaResolver() : Resolver("/schema")
    {
        resolve(
            GET, "", [=](const Request &request)
            {
                json response;
                response["result"] = SchemaService::shared()->entities();
                return Response::json(response.dump());
            });
    }
};