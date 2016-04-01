#include <boost/test/unit_test.hpp>
#include "String.hpp"
#include <iostream>

BOOST_AUTO_TEST_SUITE(TestString)

#define CHECK_VEC(a, b) { \
    auto &&a2 = a; \
    auto &&b2 = b; \
    BOOST_CHECK_EQUAL_COLLECTIONS(a2.begin(), a2.end(), b2.begin(), b2.end()); \
}
BOOST_AUTO_TEST_CASE(split)
{
    typedef std::vector<std::string> vec;

    CHECK_VEC((vec{ "a","bb","cccc" }), splitString("a,bb,cccc", ','));
    CHECK_VEC((vec{ "a","bb","" }), splitString("a,bb,", ','));
    CHECK_VEC((vec{ "","bb","cccc" }), splitString(",bb,cccc", ','));
    CHECK_VEC((vec{ "a","bb" }), splitString("a,bb", ','));
    CHECK_VEC((vec{ "a","bb" }), splitString("a,bb", ','));
    CHECK_VEC((vec{ "" }), splitString("", ','));
    CHECK_VEC((vec{ "","" }), splitString(",", ','));
}

BOOST_AUTO_TEST_SUITE_END()

