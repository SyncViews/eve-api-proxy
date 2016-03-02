#define BOOST_TEST_MODULE MyTest
#include <boost/test/unit_test.hpp>
#include "Test.hpp"

BOOST_AUTO_TEST_CASE (example_test)
{
    BOOST_CHECK_EQUAL(55, test_function());
}
