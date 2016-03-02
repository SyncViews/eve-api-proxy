#pragma once
#include "HttpHeaders.hpp"
namespace http
{
    class HttpRequest
    {
    public:
        std::string method;
        std::string path;
        HttpHeaders headers;
    };
}
