#pragma once
#include <exception>
#include "../libs/json/json.hpp"

using namespace nlohmann;

class HttpException : public std::exception
{
public:
    HttpException(const Response &response)
        : m_dump(response.body) {}

    virtual const char *what() const noexcept override
    {
        return m_dump.c_str();
    }

private:
    std::string m_dump;
};