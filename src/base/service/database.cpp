#include "database.hpp"

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/options/find.hpp>
#include <mongocxx/uri.hpp>

using namespace nlohmann;

static mongocxx::database db;

// Service
// ==============================================================================

// Service Engine
// ==============================================================================

void MongoDB::init(const std::string &connection, const std::string &dbname)
{
    static mongocxx::instance inst{};
    static mongocxx::client conn{
            mongocxx::uri{connection}};
    db = conn[dbname];
}

std::vector<std::string> MongoDB::collections()
{
    std::vector<std::string> collections;
    auto cursor = db.list_collections();
    for (const auto &doc : cursor)
    {
        auto name = doc["name"];
        collections.push_back(name.get_utf8().value.to_string());
    }
    return collections;
}

std::vector<std::string> MongoDB::find(
    const std::string &collection,
    const json &filter)
{
    bsoncxx::document::view_or_value filter_value = bsoncxx::from_json(
        filter.dump());

    std::vector<std::string> result;
    auto cursor = db[collection].find(filter_value);
    for (auto doc : cursor)
    {
        result.push_back(bsoncxx::to_json(doc));
    }
    return result;
}

std::string MongoDB::findOne(
    const std::string &collection,
    const json &filter)
{
    bsoncxx::document::view_or_value filter_value = bsoncxx::from_json(
        filter.dump());

    std::vector<std::string> result;
    auto cursor = db[collection].find_one(filter_value);
    if (cursor)
    {
        return bsoncxx::to_json(*cursor);
    }
    return "";
}

std::string MongoDB::create(
    const std::string &collection,
    const json &document)
{
    bsoncxx::document::view_or_value document_value = bsoncxx::from_json(
        document.dump());

    std::vector<std::string> result;
    auto cursor = db[collection].insert_one(document_value);
    if (cursor)
    {
        json result = {{"_id", cursor->inserted_id().get_oid().value.to_string()}};
        return result.dump();
    }
    return "";
}

std::string MongoDB::update(
    const std::string &collection,
    const json &filter,
    const json &document)
{
    bsoncxx::document::view_or_value filter_value = bsoncxx::from_json(
        filter.dump());

    bsoncxx::document::view_or_value document_value = bsoncxx::from_json(
        document.dump());

    std::vector<std::string> result;
    auto cursor = db[collection].update_many(filter_value, document_value);
    if (cursor)
    {
        json result = {{"updated", cursor->modified_count()}};
        return result.dump();
    }
    return "";
}

std::string MongoDB::remove(
    const std::string &collection,
    const json &filter)
{
    bsoncxx::document::view_or_value filter_value = bsoncxx::from_json(
        filter.dump());

    std::vector<std::string> result;
    auto cursor = db[collection].delete_many(filter_value);
    if (cursor)
    {
        json result = {{"deleted", cursor->deleted_count()}};
        return result.dump();
    }
    return "";
}