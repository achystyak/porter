#pragma once

#include <string>
#include <map>
#include <sstream>

#include "../libs/json/json.hpp"

class Utils
{
public:
    static nlohmann::json removeOptional(const nlohmann::json &body)
    {
        nlohmann::json document;
        for (auto &item : body.items())
        {
            if (!item.value().is_null())
            {
                document[item.key()] = item.value();
            }
        }
        return document;
    }

    static std::string formatted(std::string str, std::string delim)
    {
        std::size_t pos = str.find(delim);
        if (pos != std::string::npos)
        {
            str = str.substr(0, pos);
        }
        return str;
    }

    static std::vector<std::string> split(std::string str, char delim)
    {
        std::vector<std::string> tokens;
        std::stringstream token(str);
        std::string s;
        while (getline(token, s, delim))
        {
            tokens.push_back(s);
        }
        return tokens;
    }

    static std::map<std::string, std::string> params(std::string params)
    {
        std::map<std::string, std::string> result;
        params = formatted(std::string(params), " HTTP");
        std::vector<std::string> tokens = split(params, '&');

        for (unsigned int i = 0; i < tokens.size(); ++i)
        {
            std::vector<std::string> values = split(tokens[i], '=');
            if (values.size() == 2)
            {
                result[values[0]] = values[1];
            }
        }
        return result;
    }
};