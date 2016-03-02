#define BOOST_TEST_MODULE TestEveApiProxy
#include <boost/test/unit_test.hpp>
#include "Init.hpp"

struct Startup
{
    Startup()
    {
        init();
    }
}startup;
