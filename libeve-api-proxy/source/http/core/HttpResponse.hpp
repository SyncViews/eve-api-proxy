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

        void body_str(const std::string &str)
        {
            body.assign(
                (const uint8_t*)str.data(),
                (const uint8_t*)str.data() + str.size());
        }
    };
}
