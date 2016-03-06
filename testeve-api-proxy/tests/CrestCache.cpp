#include <boost/test/unit_test.hpp>
#include <iostream>
#include "CrestCache.hpp"

BOOST_AUTO_TEST_SUITE(TestCrestCache)

BOOST_AUTO_TEST_CASE(start_stop)
{
    CrestCache cache;
}

BOOST_AUTO_TEST_CASE(test_sequential)
{
    CrestCache cache;
    {
        auto a = cache.get_now("/market/10000002/orders/buy/?type=https://public-crest.eveonline.com/types/34/");
        BOOST_CHECK_EQUAL(CrestCacheEntry::UPDATED, a.entry->status);
    }
    {
        auto a = cache.get_now("/market/10000002/orders/buy/?type=https://public-crest.eveonline.com/types/34/");
        BOOST_CHECK_EQUAL(CrestCacheEntry::HIT, a.entry->status);
    }
    {
        auto a = cache.get_now("/market/10000002/orders/sell/?type=https://public-crest.eveonline.com/types/34/");
        BOOST_CHECK_EQUAL(CrestCacheEntry::UPDATED, a.entry->status);
    }
    {
        auto a = cache.get_now("/market/10000002/orders/buy/?type=https://public-crest.eveonline.com/types/34/");
        BOOST_CHECK_EQUAL(CrestCacheEntry::HIT, a.entry->status);
    }
}

BOOST_AUTO_TEST_CASE(test_parallel)
{
    CrestCache cache;
    std::vector<CrestCache::CacheLookupFutureResults> results;
    for (int i = 0; i < 100; ++i)
    {
        std::string type = "?type=https://public-crest.eveonline.com/types/" + std::to_string(34 + i) + "/";
        results.push_back(cache.get_future("/market/10000002/orders/buy/" + type));
        results.push_back(cache.get_future("/market/10000002/orders/sell/" + type));
        results.push_back(cache.get_future("/market/10000002/orders/sell/" + type));
    }

    for (auto &result : results) result.wait();

}


BOOST_AUTO_TEST_SUITE_END()

