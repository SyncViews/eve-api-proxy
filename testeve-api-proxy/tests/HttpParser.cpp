#include <boost/test/unit_test.hpp>
#include "http/core/HttpParser.hpp"
#include "Error.hpp"
#include <iostream>

BOOST_AUTO_TEST_SUITE(TestHttpParser)

BOOST_AUTO_TEST_CASE(response)
{
    http::HttpParser parser(false);
    BOOST_CHECK(!parser.is_completed());

    std::string response = 
        "HTTP/1.1 200 OK\r\n"
        "Date: Sun, 06 Mar 2016 12:00:13 GMT\r\n"
        "Content-Type: application/json; charset=UTF-8\r\n"
        "Content-Length: 23\r\n"
        "Connection: close\r\n"
        "\r\n"
        "{\"test\": \"Hello World\"}"
        ;
    auto len = parser.read((const uint8_t*)response.data(), response.size());
    BOOST_CHECK_EQUAL(response.size(), len);
    BOOST_CHECK(parser.is_completed());
    BOOST_CHECK_EQUAL(200, parser.get_status_code());
    BOOST_CHECK_EQUAL("OK", parser.get_status_message());

    auto &body = parser.get_body();
    BOOST_CHECK_EQUAL("{\"test\": \"Hello World\"}", std::string((const char*)body.data(), body.size()));

    auto &headers = parser.get_headers();
    BOOST_CHECK_EQUAL("Sun, 06 Mar 2016 12:00:13 GMT", headers.get("Date"));
    BOOST_CHECK_EQUAL("application/json; charset=UTF-8", headers.get("Content-Type"));
}

BOOST_AUTO_TEST_CASE(request)
{
    http::HttpParser parser(true);
    BOOST_CHECK(!parser.is_completed());

    std::string response =
        "GET /index.html HTTP/1.1\r\n"
        "Host: willnewbery.co.uk\r\n"
        "\r\n"
        ;
    auto len = parser.read((const uint8_t*)response.data(), response.size());
    BOOST_CHECK_EQUAL(response.size(), len);
    BOOST_CHECK(parser.is_completed());
    BOOST_CHECK_EQUAL("GET", parser.get_method());
    BOOST_CHECK_EQUAL("/index.html", parser.get_url());

    auto &headers = parser.get_headers();
    BOOST_CHECK_EQUAL("willnewbery.co.uk", headers.get("Host"));
}

BOOST_AUTO_TEST_SUITE_END()

