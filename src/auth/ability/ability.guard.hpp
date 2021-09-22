#pragma once

#include "../../api/user/user.service.hpp"
#include "../../base/auth/ability/ability.guard.hpp"

using namespace DB_User;

// Abilities Builder
// ======================================================================================================

class DefaultAbilityGuard : public AbilityGuard
{
public:
    virtual void buildAbilities(AbilityFactory *factory, const json &session) override
    {
        User user = UserService::shared()->findOne(session);
        if (user.permissions.get())
        {
            auto permissions = *user.permissions;
            for (const Permission &permission : permissions)
            {
                if (permission.domain.get() &&
                    permission.actions.get() &&
                    permission.type.get())
                {
                }
                std::string type = *permission.type;
                Domain::Type domain = *permission.domain;
                auto actions = *permission.actions;
                json filter = permission.filter.get() ? *permission.filter : json();

                for (const Action::Type &action : actions)
                {
                    if (type == "Can")
                        factory->can(user.id, action, domain, filter);

                    else if (type == "Cannot")
                        factory->cannot(user.id, action, domain, filter);
                }
            }
        }
    }
};