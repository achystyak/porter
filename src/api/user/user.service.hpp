#pragma once
#include "../../base/service/service.hpp"
#include "../../base/libs/json/json.hpp"
#include "../../base/common/shared.hpp"

#include "user.hpp"

class UserService : public Service<DB_User::User>
{
    SERVICE(UserService, Domain::User, "users")
};