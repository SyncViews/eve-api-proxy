#include <boost/test/unit_test.hpp>
#include <iostream>
#include "CrestCache.hpp"

BOOST_AUTO_TEST_SUITE(TestCrestCache)

BOOST_AUTO_TEST_CASE(start_stop)
{
    CrestCache cache;
}

BOOST_AUTO_TEST_CASE(test)
{
    CrestCache cache;
    {
        auto a = cache.get("/market/10000002/orders/buy/?type=https://public-crest.eveonline.com/types/34/");
        BOOST_CHECK_EQUAL(CrestCacheEntry::UPDATED, a.entry->status);
    }
    {
        auto a = cache.get("/market/10000002/orders/buy/?type=https://public-crest.eveonline.com/types/34/");
        BOOST_CHECK_EQUAL(CrestCacheEntry::HIT, a.entry->status);
    }
    {
        auto a = cache.get("/market/10000002/orders/sell/?type=https://public-crest.eveonline.com/types/34/");
        BOOST_CHECK_EQUAL(CrestCacheEntry::UPDATED, a.entry->status);
    }
    {
        auto a = cache.get("/market/10000002/orders/buy/?type=https://public-crest.eveonline.com/types/34/");
        BOOST_CHECK_EQUAL(CrestCacheEntry::HIT, a.entry->status);
    }
}


BOOST_AUTO_TEST_SUITE_END()

