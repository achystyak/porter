#pragma once

#include <string>
#include <vector>

#include "../libs/json/json.hpp"

using namespace nlohmann;

class MongoDB
{
public:
    static void init(const std::string &connection, const std::string &dbname);

    static std::vector<std::string> collections();

    static std::vector<std::string> find(const std::string &collection, const json &filter);

    static std::string findOne(const std::string &collection, const json &filter);

    static std::string create(const std::string &collection, const json &document);

    static std::string update(const std::string &collection, const json &filter, const json &document);

    static std::string remove(const std::string &collection, const json &filter);
};