#pragma once

#include <string>
#include "../common/http.hpp"

class Curl
{
public:
    typedef std::map<std::string, std::string> Headers;

    static Response Get(
        const std::string &uri,
        std::string body = "",
        Headers headers = Headers());

    static Response Post(
        const std::string &uri,
        std::string body = "",
        Headers headers = Headers());

    static Response Put(
        const std::string &uri,
        std::string body = "",
        Headers headers = Headers());

    static Response Patch(
        const std::string &uri,
        std::string body = "",
        Headers headers = Headers());

    static Response Delete(
        const std::string &uri,
        std::string body = "",
        Headers headers = Headers());

private:
    Curl();
    Curl(const Curl &curl);
};