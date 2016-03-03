#include <boost/test/unit_test.hpp>
#include "socket/TlsSocket.hpp"
#include "http/CrestRequest.hpp"
#include "http/core/HttpResponseParser.hpp"
#include "Error.hpp"
#include "Gzip.hpp"
#include <iostream>

BOOST_AUTO_TEST_SUITE(socket_Crest)

BOOST_AUTO_TEST_CASE(index)
{
    std::cout << "GET https://public-crest.eveonline.com/" << std::endl;
    TlsSocket sock;
    sock.connect("public-crest.eveonline.com", 443);
    send_crest_get_request(&sock, "/");

    http::HttpResponseParser response;
    while (!response.is_completed())
    {
        uint8_t buffer[4096];
        auto len = sock.recv(buffer, sizeof(buffer));
        auto len2 = response.read(buffer, len);
        assert(len == len2);
    }

    std::cout << response.get_status_code() << " " << response.get_status_message() << std::endl;
    for (auto &i : response.get_headers().headers)
    {
        std::cout << i.name << ": " << i.value << std::endl;
    }
    auto body = gzip_decompress(response.get_body());
    std::cout.write((const char*)body.data(), body.size());
    std::cout << std::endl;
    std::cout << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
