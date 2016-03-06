#include <boost/test/unit_test.hpp>
#include "http/core/UrlParser.hpp"
#include <iostream>

BOOST_AUTO_TEST_SUITE(TestUrlParser)

BOOST_AUTO_TEST_CASE(test)
{
    auto x = http::UrlParser("https://willnewbery.co.uk/");
    BOOST_CHECK_EQUAL("https", x.protocol);
    BOOST_CHECK_EQUAL("willnewbery.co.uk", x.host);
    BOOST_CHECK_EQUAL("", x.port);
    BOOST_CHECK_EQUAL("/", x.path);
    BOOST_CHECK_EQUAL(0, x.query_params.size());

    x = http::UrlParser("https://willnewbery.co.uk:70/");
    BOOST_CHECK_EQUAL("https", x.protocol);
    BOOST_CHECK_EQUAL("willnewbery.co.uk", x.host);
    BOOST_CHECK_EQUAL("70", x.port);
    BOOST_CHECK_EQUAL("/", x.path);
    BOOST_CHECK_EQUAL(0, x.query_params.size());

    x = http::UrlParser("https://willnewbery.co.uk/test?param=55");
    BOOST_CHECK_EQUAL("https", x.protocol);
    BOOST_CHECK_EQUAL("willnewbery.co.uk", x.host);
    BOOST_CHECK_EQUAL("", x.port);
    BOOST_CHECK_EQUAL("/test", x.path);
    BOOST_CHECK_EQUAL("55", x.query_param("param"));

    x = http::UrlParser("https://willnewbery.co.uk/test?param=55&bar");
    BOOST_CHECK_EQUAL("https", x.protocol);
    BOOST_CHECK_EQUAL("willnewbery.co.uk", x.host);
    BOOST_CHECK_EQUAL("", x.port);
    BOOST_CHECK_EQUAL("/test", x.path);
    BOOST_CHECK_EQUAL("55", x.query_param("param"));
    BOOST_CHECK_EQUAL("", x.query_param("bar"));

    x = http::UrlParser("https://willnewbery.co.uk/test?arr=55&arr=75");
    BOOST_CHECK_EQUAL("https", x.protocol);
    BOOST_CHECK_EQUAL("willnewbery.co.uk", x.host);
    BOOST_CHECK_EQUAL("", x.port);
    BOOST_CHECK_EQUAL("/test", x.path);
    auto arr = x.query_array_param("arr");
    BOOST_CHECK_EQUAL(2, arr.size());
    BOOST_CHECK_EQUAL("55", arr[0]);
    BOOST_CHECK_EQUAL("75", arr[1]);


    x = http::UrlParser("/");
    BOOST_CHECK_EQUAL("/", x.path);
    BOOST_CHECK_EQUAL(0, x.query_params.size());

    x = http::UrlParser("/test?arr=55&arr=75");
    BOOST_CHECK_EQUAL("/test", x.path);
    auto arr2 = x.query_array_param("arr");
    BOOST_CHECK_EQUAL(2, arr2.size());
    BOOST_CHECK_EQUAL("55", arr2[0]);
    BOOST_CHECK_EQUAL("75", arr2[1]);
}

BOOST_AUTO_TEST_SUITE_END()

