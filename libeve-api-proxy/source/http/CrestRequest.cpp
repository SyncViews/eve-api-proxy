#include "Precompiled.hpp"
#include "CrestRequest.hpp"
#include "../socket/Socket.hpp"
#include <string>
#include <iostream>

void send_crest_get_request(Socket *socket, const std::string &path)
{
    std::string req =
        "GET " + path + " HTTP/1.1\r\n"
        "Connection: close\r\n"
        "Host: public-crest.eveonline.com\r\n"
        "User-Agent: eve-api-proxy server admin@willnewbery.co.uk\r\n"
        "Accept-Encoding: gzip\r\n"
        "\r\n";
    socket->send_all((const uint8_t*)req.data(), req.size());
}
