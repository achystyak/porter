#pragma once
#include "../../base/resolver/resolver.hpp"
#include "../../base/libs/json/json.hpp"

#include "user.service.hpp"
#include "user.hpp"
#include "user.inputs.hpp"

using namespace nlohmann;

class UserResolver : public Resolver
{
public:
    UserResolver() : Resolver("/users")
    {
        resolve(
            GET, "/find", [=](const Request &request)
            { return this->findById(request); },
            RO_AUTH_GUARDED, {Action::Read, Domain::User});

        resolve(
            GET, "/list", [=](const Request &request)
            { return this->find(request); },
            RO_AUTH_GUARDED, {Action::Read, Domain::User});

        resolve(
            POST, "", [=](const Request &request)
            { return this->create(request); },
            RO_AUTH_GUARDED, {Action::Create, Domain::User});

        resolve(
            PATCH, "", [=](const Request &request)
            { return this->updateById(request); },
            RO_AUTH_GUARDED, {Action::Update, Domain::User});

        resolve(
            DELETE, "", [=](const Request &request)
            { return this->removeById(request); },
            RO_AUTH_GUARDED, {Action::Delete, Domain::User});
    }

    Response find(const Request &request)
    {
        FindUserInput input;
        if (input.parse(request.body))
        {
            std::vector<DB_User::User> users = UserService::shared()->find(input.toDocument());
            json result = UserService::shared()->ableFilter(request.session, Action::Read, users);

            return Response::json(result.dump());
        }
        return Response::badRequest();
    }

    Response findById(const Request &request)
    {
        FindOneUserInput input;
        if (input.parse(request.queries))
        {
            DB_User::User user = UserService::shared()->findOne(input.toFilter());

            if (user.id.get())
            {
                json result = user;
                return Response::json(result.dump());
            }
            return Response::notFound();
        }
        return Response::badRequest();
    }

    Response create(const Request &request)
    {
        CreateUserInput input;
        if (input.parse(request.body))
        {
            json result = UserService::shared()->create(input.entity);

            return Response::json(result.dump());
        }
        return Response::badRequest();
    }

    Response updateById(const Request &request)
    {
        UpdateUserInput input;
        if (input.parse(request.body))
        {
            json result = UserService::shared()->update(input.toFilter(), input.toDocument());

            return Response::json(result.dump());
        }
        return Response::badRequest();
    }

    Response removeById(const Request &request)
    {
        DeleteUserInput input;
        if (input.parse(request.body))
        {
            json result = UserService::shared()->remove(input.toFilter());

            return Response::json(result.dump());
        }
        return Response::badRequest();
    }
};