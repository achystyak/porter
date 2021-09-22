#pragma once

#include "../../libs/json/json.hpp"
#include "ability.factory.hpp"

using namespace nlohmann;

class AbilityGuard
{
public:
    AbilityGuard() {}
    virtual ~AbilityGuard() {}

    virtual void buildAbilities(AbilityFactory *factory, const json &session) = 0;
};