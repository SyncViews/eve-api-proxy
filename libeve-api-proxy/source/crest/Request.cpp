#include "Precompiled.hpp"
#include "Request.hpp"
#include "../socket/Socket.hpp"
#include <string>
#include <iostream>

namespace crest
{
    void send_get_request(Socket *socket, const std::string &path)
    {
        std::string req =
            "GET " + path + " HTTP/1.1\r\n"
            "Connection: keep-alive\r\n"
            "Host: crest-tq.eveonline.com\r\n"
            "User-Agent: eve-api-proxy server admin@willnewbery.co.uk\r\n"
            "Accept-Encoding: gzip\r\n"
            "\r\n";
        socket->send_all((const uint8_t*)req.data(), req.size());
    }
}

