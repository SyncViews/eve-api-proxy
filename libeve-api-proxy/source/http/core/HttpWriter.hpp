#pragma once
#include "HttpHeader.hpp"
#include "HttpHeaders.hpp"
#include "HttpRequest.hpp"
#include <string>
#include <ostream>
namespace http
{
    inline void write_http_header(std::ostream &os, const HttpHeader &header)
    {
        os << header.name << ": " << header.value << "\r\n";
    }
    inline void write_http_headers(std::ostream &os, const HttpHeaders &headers)
    {
        for (auto &header : headers.headers) write_http_header(os, header);
    }
}
