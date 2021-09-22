#pragma once
#include "../../base/inputs/inputs.hpp"
#include "../../base/libs/json/json.hpp"

#include "user.hpp"

using namespace nlohmann;

// Input for finding user by id
class FindOneUserInput : public FindOneInput
{
};

// Input for finding array of users
class FindUserInput : public FindInput<DB_User::User>
{
};

// Input for creating single user
class CreateUserInput : public CreateInput<DB_User::User>
{
};

// Input for updating users by filter
class UpdateUserInput : public UpdateInput<DB_User::User>
{
};

// Input for deleting users by filter
class DeleteUserInput : public DeleteInput
{
};