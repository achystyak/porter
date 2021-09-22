#pragma once
#include <exception>
#include "../libs/json/json.hpp"
#include "../common/http.hpp"
#include "../common/utils.hpp"
#include "../logger/logger.hpp"

using namespace nlohmann;

template <class Entity>
class CreateInput
{
public:
    CreateInput() {}
    virtual ~CreateInput() {}

    virtual bool parse(const std::string &data)
    {
        try
        {
            if (json::accept(data))
            {
                entity = json::parse(data);
                return true;
            }
        }
        catch (const std::exception &exc)
        {
            Log_Error("%s", exc.what());
        }
        return false;
    }

    virtual json toDocument()
    {
        json document = Utils::removeOptional(entity);
        return document;
    }

    Entity entity;
};

template <class Entity>
class UpdateInput
{
public:
    UpdateInput() {}
    virtual ~UpdateInput() {}

    virtual bool parse(const std::string &data)
    {
        try
        {
            if (json::accept(data))
            {
                json body = json::parse(data);
                if (body.contains("id") && body["id"].is_string() &&
                    body.contains("data") && body["data"].is_object())
                {
                    id = body.at("id").get<std::string>();
                    entity = body["data"];
                    return true;
                }
            }
        }
        catch (const std::exception &exc)
        {
            Log_Error("%s", exc.what());
        }
        return false;
    }

    virtual json toFilter()
    {
        json filter = {{"_id", {{"$oid", id}}}};
        return filter;
    }

    virtual json toDocument()
    {
        json document = Utils::removeOptional(entity);
        return document;
    }

    Entity entity;
    std::string id;
};

class DeleteInput
{
public:
    DeleteInput() {}
    virtual ~DeleteInput() {}

    virtual bool parse(const std::string &data)
    {
        try
        {
            if (json::accept(data))
            {
                json body = json::parse(data);
                if (body.contains("id") && body["id"].is_string())
                {
                    id = body.at("id").get<std::string>();
                    return true;
                }
            }
        }
        catch (const std::exception &exc)
        {
            Log_Error("%s", exc.what());
        }
        return false;
    }

    virtual json toFilter()
    {
        json filter = {{"_id", {{"$oid", id}}}};
        return filter;
    }

    std::string id;
};

class FindOneInput : public DeleteInput
{
public:
    FindOneInput() {}
    virtual ~FindOneInput() {}

    virtual bool parse(const std::map<std::string, std::string> &queries)
    {
        try
        {
            json body = queries;
            return DeleteInput::parse(body.dump());
        }
        catch (const std::exception &exc)
        {
            Log_Error("%s", exc.what());
        }
        return false;
    }
};

template <class Entity>
class FindInput : public CreateInput<Entity>
{
public:
    FindInput() {}
    virtual ~FindInput() {}
};
