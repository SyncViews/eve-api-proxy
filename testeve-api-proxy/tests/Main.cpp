#define BOOST_TEST_MODULE TestEveApiProxy
#include <boost/test/unit_test.hpp>
#include "Init.hpp"
#include <http/net/Net.hpp>

struct Startup
{
    Startup()
    {
        init();
    }
};
BOOST_GLOBAL_FIXTURE(Startup);

