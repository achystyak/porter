#pragma once

#include "../../common/enums.hpp"
#include "../../libs/json/json.hpp"

using namespace nlohmann;

class Ability
{
public:
    Ability()
        : action(Action::None),
          domain(Domain::None),
          filter(json()) {}

    Ability(
        std::string _action,
        std::string _domain,
        json _filter = json())
        : action(_action),
          domain(_domain),
          filter(_filter) {}

    bool isEmpty() const
    {
        return (action == Action::None || domain == Domain::None);
    }

    Action::Type action;
    Domain::Type domain;
    json filter;
};

inline bool operator==(const Ability &lhs, const Ability &rhs)
{
    return lhs.action == rhs.action &&
           lhs.domain == rhs.domain;
}