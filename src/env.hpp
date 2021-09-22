#pragma once

#include <sstream>
#include <fstream>
#include <string>
#include <filesystem>
#include <iostream>
#include <exception>

#include "base/libs/json/json.hpp"
#include "base/logger/logger.hpp"

using namespace nlohmann;

static json env;

json loadEnv()
{
    try
    {
        std::string path = std::filesystem::current_path();
        path += "/config.json";
        std::ifstream t(path);
        std::stringstream buffer;
        buffer << t.rdbuf();
        env = json::parse(buffer.str());
    }
    catch (const std::exception &exc)
    {
        Log_Error("%s", exc.what());
        throw exc;
    }
    return env;
}