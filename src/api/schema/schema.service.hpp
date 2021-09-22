#pragma once
#include "../../base/service/service.hpp"
#include "../../base/libs/json/json.hpp"
#include "../../base/common/shared.hpp"

using namespace nlohmann;

class SchemaService : public Service<json>
{
    SERVICE(SchemaService, Domain::Schema, "utils")

    std::map<std::string, json> entities()
    {
        std::map<std::string, json> entities;
        auto collections = MongoDB::collections();

        for (const std::string &collection : collections)
        {
            auto entity = MongoDB::findOne(collection, {{}});
            if (entity.size() && json::accept(entity))
            {
                entities[collection] = json::parse(entity);
            }
        }
        return entities;
    }
};