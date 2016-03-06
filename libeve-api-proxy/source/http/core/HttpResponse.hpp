#pragma once
#include "HttpHeaders.hpp"
namespace http
{
    class HttpResponse
    {
    public:
        int status_code;
        HttpHeaders headers;
        std::vector<uint8_t> body;
    };
}
