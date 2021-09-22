#pragma once
#include "ability.hpp"
#include "../../common/shared.hpp"
#include "../../logger/logger.hpp"

static std::vector<Ability> filterAbilities(
    const json &user,
    const Ability &ability,
    const std::map<json, std::vector<Ability>> &abilities,
    bool conditioned)
{
    std::vector<Ability> result;
    for (auto able : abilities)
    {
        if (able.first == user)
        {
            for (auto ab : able.second)
            {
                if (ab == ability ||
                    ab.domain == Domain::All && (ab.action == Action::All || ab.action == ability.action) ||
                    ab.action == Domain::All && ab.domain == ability.domain)
                {
                    if (!conditioned || ab.filter.is_null() == false)
                    {
                        result.push_back(ab);
                    }
                }
            }
        }
    }
    return result;
}

static bool isAble(
    const json &user,
    const Ability &ability,
    const std::map<json, std::vector<Ability>> &abilities)
{
    for (auto able : abilities)
    {
        if (able.first == user)
        {
            for (auto ab : able.second)
            {
                if (ab == ability ||
                    ab.domain == Domain::All && (ab.action == Action::All || ab.action == ability.action) ||
                    ab.action == Domain::All && ab.domain == ability.domain)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

class AbilityFactory
{
public:
    static AbilityFactory *shared()
    {
        static AbilityFactory *instance = new AbilityFactory();
        return instance;
    }

    AbilityFactory *can(const json &userId, Action::Type action, Domain::Type domain, json filter = json())
    {
        if (m_can_abilities.find(userId) == m_can_abilities.end())
        {
            m_can_abilities[userId] = std::vector<Ability>();
        }
        m_can_abilities[userId].push_back(Ability(action, domain, filter));
        return this;
    }

    AbilityFactory *cannot(const json &userId, Action::Type action, Domain::Type domain, json filter = json())
    {
        if (m_cannot_abilities.find(userId) == m_cannot_abilities.end())
        {
            m_cannot_abilities[userId] = std::vector<Ability>();
        }
        m_cannot_abilities[userId].push_back(Ability(action, domain, filter));
        return this;
    }

    bool verify(const json &userId, const Ability &ability) const
    {
        if (ability.isEmpty())
        {
            return true;
        }

        if (isAble(userId, ability, m_cannot_abilities))
        {
            return false;
        }

        if (m_can_abilities.find(userId) == m_can_abilities.end() ||
            m_can_abilities.at(userId).empty())
        {
            return true;
        }

        return isAble(userId, ability, m_can_abilities);
    }

    std::vector<Ability> canAbilities(const json &userId, Action::Type action, Domain::Type domain, bool conditioned = false)
    {
        return filterAbilities(userId, {action, domain}, m_can_abilities, conditioned);
    }

    std::vector<Ability> cannotAbilities(const json &userId, Action::Type action, Domain::Type domain, bool conditioned = false)
    {
        return filterAbilities(userId, {action, domain}, m_cannot_abilities, conditioned);
    }

private:
    AbilityFactory() {}
    AbilityFactory(const AbilityFactory &a) {}

    std::map<json, std::vector<Ability>> m_can_abilities;
    std::map<json, std::vector<Ability>> m_cannot_abilities;
};