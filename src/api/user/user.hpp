//  To parse this JSON data, first install
//
//      json.hpp  https://github.com/nlohmann/json
//
//  Then include this file, and then do
//
//     User data = nlohmann::json::parse(jsonString);

#pragma once

#include "../../base/libs/json/json.hpp"

#ifndef NLOHMANN_OPT_HELPER
#define NLOHMANN_OPT_HELPER
namespace nlohmann
{
    template <typename T>
    struct adl_serializer<std::shared_ptr<T>>
    {
        static void to_json(json &j, const std::shared_ptr<T> &opt)
        {
            if (!opt)
                j = nullptr;
            else
                j = *opt;
        }

        static std::shared_ptr<T> from_json(const json &j)
        {
            if (j.is_null())
                return std::unique_ptr<T>();
            else
                return std::unique_ptr<T>(new T(j.get<T>()));
        }
    };
}
#endif

namespace DB_User
{
    using nlohmann::json;

    inline json get_untyped(const json &j, const char *property)
    {
        if (j.find(property) != j.end())
        {
            return j.at(property).get<json>();
        }
        return json();
    }

    inline json get_untyped(const json &j, std::string property)
    {
        return get_untyped(j, property.data());
    }

    template <typename T>
    inline std::shared_ptr<T> get_optional(const json &j, const char *property)
    {
        if (j.find(property) != j.end())
        {
            return j.at(property).get<std::shared_ptr<T>>();
        }
        return std::shared_ptr<T>();
    }

    template <typename T>
    inline std::shared_ptr<T> get_optional(const json &j, std::string property)
    {
        return get_optional<T>(j, property.data());
    }

    struct Id
    {
        std::shared_ptr<std::string> oid;
    };

    struct Permission
    {
        std::shared_ptr<std::string> type;
        std::shared_ptr<std::string> domain;
        std::shared_ptr<std::vector<std::string>> actions;
        std::shared_ptr<json> filter;
    };

    struct User
    {
        std::shared_ptr<std::string> email;
        std::shared_ptr<std::string> password;
        std::shared_ptr<std::string> first_name;
        std::shared_ptr<std::string> last_name;
        std::shared_ptr<std::vector<Permission>> permissions;
        std::shared_ptr<Id> id;
    };
}

namespace nlohmann
{
    void from_json(const json &j, DB_User::Id &x);
    void to_json(json &j, const DB_User::Id &x);

    void from_json(const json &j, json &x);
    void to_json(json &j, const json &x);

    void from_json(const json &j, DB_User::Permission &x);
    void to_json(json &j, const DB_User::Permission &x);

    void from_json(const json &j, DB_User::User &x);
    void to_json(json &j, const DB_User::User &x);

    inline void from_json(const json &j, DB_User::Id &x)
    {
        x.oid = DB_User::get_optional<std::string>(j, "$oid");
    }

    inline void to_json(json &j, const DB_User::Id &x)
    {
        j = json::object();
        j["$oid"] = x.oid;
    }

    inline void from_json(const json &j, json &x)
    {
    }

    inline void to_json(json &j, const json &x)
    {
        j = json::object();
    }

    inline void from_json(const json &j, DB_User::Permission &x)
    {
        x.type = DB_User::get_optional<std::string>(j, "type");
        x.domain = DB_User::get_optional<std::string>(j, "domain");
        x.actions = DB_User::get_optional<std::vector<std::string>>(j, "actions");
        x.filter = DB_User::get_optional<json>(j, "filter");
    }

    inline void to_json(json &j, const DB_User::Permission &x)
    {
        j = json::object();
        j["type"] = x.type;
        j["domain"] = x.domain;
        j["actions"] = x.actions;
        j["filter"] = x.filter;
    }

    inline void from_json(const json &j, DB_User::User &x)
    {
        x.email = DB_User::get_optional<std::string>(j, "email");
        x.password = DB_User::get_optional<std::string>(j, "password");
        x.first_name = DB_User::get_optional<std::string>(j, "firstName");
        x.last_name = DB_User::get_optional<std::string>(j, "lastName");
        x.permissions = DB_User::get_optional<std::vector<DB_User::Permission>>(j, "permissions");
        x.id = DB_User::get_optional<DB_User::Id>(j, "_id");
    }

    inline void to_json(json &j, const DB_User::User &x)
    {
        j = json::object();
        j["email"] = x.email;
        j["password"] = x.password;
        j["firstName"] = x.first_name;
        j["lastName"] = x.last_name;
        j["permissions"] = x.permissions;
        j["_id"] = x.id;
    }
}
