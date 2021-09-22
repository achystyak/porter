#include "curl.hpp"
#include <cpr/cpr.h>

size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string *data)
{
    data->append((char *)ptr, size * nmemb);
    return size * nmemb;
}

Response Curl::Get(
    const std::string &uri,
    std::string body,
    Headers headers)
{
    cpr::Header header;
    header.insert(headers.begin(), headers.end());

    cpr::Response r = cpr::Get(
        cpr::Url{uri},
        cpr::Body{body},
        header);

    return Response((HttpStatus)r.status_code, r.text, "");
}

Response Curl::Post(
    const std::string &uri,
    std::string body,
    Headers headers)
{
    cpr::Header header;
    header.insert(headers.begin(), headers.end());

    cpr::Response r = cpr::Post(
        cpr::Url{uri},
        cpr::Body{body},
        header);

    return Response((HttpStatus)r.status_code, r.text, "");
}

Response Curl::Put(
    const std::string &uri,
    std::string body,
    Headers headers)
{
    cpr::Header header;
    header.insert(headers.begin(), headers.end());

    cpr::Response r = cpr::Put(
        cpr::Url{uri},
        cpr::Body{body},
        header);

    return Response((HttpStatus)r.status_code, r.text, "");
}

Response Curl::Patch(
    const std::string &uri,
    std::string body,
    Headers headers)
{
    cpr::Header header;
    header.insert(headers.begin(), headers.end());

    cpr::Response r = cpr::Patch(
        cpr::Url{uri},
        cpr::Body{body},
        header);

    return Response((HttpStatus)r.status_code, r.text, "");
}

Response Curl::Delete(
    const std::string &uri,
    std::string body,
    Headers headers)
{
    cpr::Header header;
    header.insert(headers.begin(), headers.end());

    cpr::Response r = cpr::Delete(
        cpr::Url{uri},
        cpr::Body{body},
        header);

    return Response((HttpStatus)r.status_code, r.text, "");
}