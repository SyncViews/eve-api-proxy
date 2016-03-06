#pragma once
#include "HttpHeaders.hpp"
#include "UrlParser.hpp"
namespace http
{
    class HttpRequest
    {
    public:
        std::string method;
        UrlParser url;
        HttpHeaders headers;
    };
}
