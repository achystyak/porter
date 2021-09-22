#pragma once
#include <exception>

#include "database.hpp"
#include "../common/http.hpp"
#include "../common/enums.hpp"
#include "../libs/mongoose/mongoose.hpp"
#include "../common/utils.hpp"
#include "../common/exceptions.hpp"
#include "../logger/logger.hpp"
#include "../auth/ability/ability.factory.hpp"

using namespace nlohmann;

template <class Entity>
class Service
{
public:
    Service(Domain::Type domain, const std::string &collection)
        : m_domain(domain),
          m_collection(collection) {}

    virtual ~Service() {}

    virtual Entity findOne(const json &filter)
    {
        Entity entity;
        try
        {
            std::string result = MongoDB::findOne(m_collection, filter);
            entity = json::parse(result);
        }
        catch (const std::exception &exc)
        {
            Log_Error("%s", exc.what());
            throw HttpException(Response::notFound());
        }
        return entity;
    }

    virtual std::vector<Entity> find(const json &filter)
    {
        std::vector<Entity> entities;
        try
        {
            std::vector<std::string> result = MongoDB::find(m_collection, filter);
            for (const std::string &entry : result)
            {
                if (json::accept(entry))
                {
                    Entity entity = json::parse(entry);
                    entities.push_back(entity);
                }
            }
        }
        catch (const std::exception &exc)
        {
            Log_Error("%s", exc.what());
            throw HttpException(Response::notFound());
        }

        return entities;
    }

    virtual const json create(const Entity &entity)
    {
        try
        {
            json document = Utils::removeOptional(entity);
            std::string result = MongoDB::create(m_collection, document);
            json body = json::parse(result);
            return body;
        }
        catch (const std::exception &exc)
        {
            Log_Error("%s", exc.what());
            throw HttpException(Response::badRequest());
        }
    }

    virtual json update(const json &filter, const Entity &entity)
    {
        try
        {
            json document;
            document["$set"] = Utils::removeOptional(entity);
            std::string result = MongoDB::update(m_collection, filter, document);
            return json::parse(result);
        }
        catch (const std::exception &exc)
        {
            Log_Error("%s", exc.what());
            throw HttpException(Response::badRequest());
        }
    }

    virtual const json remove(const json &filter)
    {
        try
        {
            std::string result = MongoDB::remove(m_collection, filter);
            return json::parse(result);
        }
        catch (const std::exception &exc)
        {
            Log_Error("%s", exc.what());
            throw HttpException(Response::badRequest());
        }
    }

    // Abilities Section
    // =======================================================================================================================

    std::vector<Entity> ableAll(const json &session, Action::Type action, std::vector<Entity> payload)
    {
        if (session.contains("_id") == false)
            return payload;

        auto canAbilities = AbilityFactory::shared()->canAbilities(session["_id"], action, m_domain, true);
        auto cannotAbilities = AbilityFactory::shared()->canAbilities(session["_id"], action, m_domain, true);

        for (const Entity &entity : payload)
            if (isAble(cannotAbilities, entity))
                return std::vector<Entity>();

        if (canAbilities.empty())
            return payload;

        for (const Entity &entity : payload)
            if (!isAble(canAbilities, entity))
                return std::vector<Entity>();

        return payload;
    }

    std::vector<Entity> ableFilter(const json &session, Action::Type action, std::vector<Entity> payload)
    {
        if (session.contains("_id") == false)
            return payload;

        std::vector<Entity> result;

        auto canAbilities = AbilityFactory::shared()->canAbilities(session["_id"], action, m_domain, true);
        auto cannotAbilities = AbilityFactory::shared()->cannotAbilities(session["_id"], action, m_domain, true);

        for (const Entity &entity : payload)
        {
            if (!isAble(cannotAbilities, entity))
            {
                if (canAbilities.empty() || isAble(canAbilities, entity))
                {
                    result.push_back(entity);
                }
            }
        }

        return result;
    }

    Entity ableOne(const json &session, Action::Type action, const Entity &payload)
    {
        auto result = ableAll(session, action, {payload});
        return result.size() ? result[0] : json();
    }

private:
    bool isAble(const std::vector<Ability> &abilities, json entity)
    {
        for (const Ability &ability : abilities)
        {
            std::vector<Entity> ables = find(ability.filter);
            for (const Entity &able : ables)
            {
                json dumped = able;
                if (dumped.contains("_id") && entity.contains("_id") &&
                    dumped["_id"].dump() == entity["_id"].dump())
                {
                    return true;
                }
            }
        }
        return false;
    }

    std::string m_collection;
    Domain::Type m_domain;
};