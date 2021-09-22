#pragma once

#include <string>
#include <map>
#include <sstream>

#include "../libs/json/json.hpp"

#define JSON_HEADERS "Content-Type: application/json; charset=utf-8\r\n"

enum HttpMethod
{
    GET,
    POST,
    PUT,
    PATCH,
    DELETE,
    UNKNOWN
};

enum HttpStatus
{
    HTTP_CONTINUE = 100,
    HTTP_SWITCHING_PROTOCOLS = 101,
    HTTP_PROCESSING = 102,
    HTTP_EARLYHINTS = 103,
    HTTP_OK = 200,
    HTTP_CREATED = 201,
    HTTP_ACCEPTED = 202,
    HTTP_NON_AUTHORITATIVE_INFORMATION = 203,
    HTTP_NO_CONTENT = 204,
    HTTP_RESET_CONTENT = 205,
    HTTP_PARTIAL_CONTENT = 206,
    HTTP_AMBIGUOUS = 300,
    HTTP_MOVED_PERMANENTLY = 301,
    HTTP_FOUND = 302,
    HTTP_SEE_OTHER = 303,
    HTTP_NOT_MODIFIED = 304,
    HTTP_TEMPORARY_REDIRECT = 307,
    HTTP_PERMANENT_REDIRECT = 308,
    HTTP_BAD_REQUEST = 400,
    HTTP_UNAUTHORIZED = 401,
    HTTP_PAYMENT_REQUIRED = 402,
    HTTP_FORBIDDEN = 403,
    HTTP_NOT_FOUND = 404,
    HTTP_METHOD_NOT_ALLOWED = 405,
    HTTP_NOT_ACCEPTABLE = 406,
    HTTP_PROXY_AUTHENTICATION_REQUIRED = 407,
    HTTP_REQUEST_TIMEOUT = 408,
    HTTP_CONFLICT = 409,
    HTTP_GONE = 410,
    HTTP_LENGTH_REQUIRED = 411,
    HTTP_PRECONDITION_FAILED = 412,
    HTTP_PAYLOAD_TOO_LARGE = 413,
    HTTP_URI_TOO_LONG = 414,
    HTTP_UNSUPPORTED_MEDIA_TYPE = 415,
    HTTP_REQUESTED_RANGE_NOT_SATISFIABLE = 416,
    HTTP_EXPECTATION_FAILED = 417,
    HTTP_I_AM_A_TEAPOT = 418,
    HTTP_MISDIRECTED = 421,
    HTTP_UNPROCESSABLE_ENTITY = 422,
    HTTP_FAILED_DEPENDENCY = 424,
    HTTP_TOO_MANY_REQUESTS = 429,
    HTTP_INTERNAL_SERVER_ERROR = 500,
    HTTP_NOT_IMPLEMENTED = 501,
    HTTP_BAD_GATEWAY = 502,
    HTTP_SERVICE_UNAVAILABLE = 503,
    HTTP_GATEWAY_TIMEOUT = 504,
    HTTP_HTTP_VERSION_NOT_SUPPORTED = 505
};

enum ResolverOptions
{
    RO_NONE = 0,
    RO_AUTH_GUARDED = 2
};

class Request
{
public:
    Request(HttpMethod _method,
            std::string _uri,
            std::string _body = "",
            std::map<std::string, std::string> _headers = std::map<std::string, std::string>(),
            std::map<std::string, std::string> _queries = std::map<std::string, std::string>())
        : body(_body),
          headers(_headers),
          method(_method),
          queries(_queries),
          uri(_uri) {}

    std::string body;
    HttpMethod method;
    std::string uri;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> queries;
    nlohmann::json session;

    static Request get(const std::string &uri)
    {
        return Request(GET, uri);
    }

    static HttpMethod parseMethod(std::string &raw)
    {
        if (raw == "GET")
            return GET;
        if (raw == "POST")
            return POST;
        if (raw == "PATCH")
            return PATCH;
        if (raw == "PUT")
            return PUT;
        if (raw == "DELETE")
            return DELETE;

        return UNKNOWN;
    }
};

class Response
{
public:
    Response(
        HttpStatus _code,
        std::string _body,
        std::string _headers = "")
        : body(_body),
          code(_code),
          headers(_headers)
    {
    }

    Response(const Response &r)
    {
        code = r.code;
        body = r.body;
        headers = r.headers;
    }

    Response &operator=(const Response &r)
    {
        if (this == &r)
            return *this;

        headers = r.headers;
        body = r.body;
        code = r.code;
        return *this;
    }

    std::string headers;
    std::string body;
    HttpStatus code;

    const static Response json(const std::string &body)
    {
        return Response(HTTP_OK, body, JSON_HEADERS);
    }

    const static Response unauthorized()
    {
        auto body = Response::jsonError(HTTP_UNAUTHORIZED, "Unauthorized").dump();
        return Response(HTTP_UNAUTHORIZED, body, JSON_HEADERS);
    }

    const static Response forbidden()
    {
        auto body = Response::jsonError(HTTP_FORBIDDEN, "Forbidden resource").dump();
        return Response(HTTP_FORBIDDEN, body, JSON_HEADERS);
    }

    const static Response notFound()
    {
        auto body = Response::jsonError(HTTP_NOT_FOUND, "Not found").dump();
        return Response(HTTP_NOT_FOUND, body, JSON_HEADERS);
    }

    const static Response badRequest()
    {
        auto body = Response::jsonError(HTTP_BAD_REQUEST, "Bad request").dump();
        return Response(HTTP_BAD_REQUEST, body, JSON_HEADERS);
    }

    const static Response internalError()
    {
        auto body = Response::jsonError(HTTP_INTERNAL_SERVER_ERROR, "Internal server error").dump();
        return Response(HTTP_INTERNAL_SERVER_ERROR, body, JSON_HEADERS);
    }

    static nlohmann::json jsonError(HttpStatus code, std::string message)
    {
        nlohmann::json json = {{"error", {{"code", code}, {"message", message}}}};
        return json;
    }
};

class Message
{
public:
    Message(
        std::string _topic,
        std::string _message)
        : message(_message),
          topic(_topic)
    {
    }

    std::string message;
    std::string topic;
};

typedef std::function<Response(const Request &)> ResponseCallback;
typedef std::function<void(const std::map<std::string, nlohmann::json> &, const std::string &)> PublishCallback;
typedef std::function<void()> ScheduleCallback;