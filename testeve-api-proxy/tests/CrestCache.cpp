#include <boost/test/unit_test.hpp>
#include <iostream>
#include "crest/CacheOld.hpp"

BOOST_AUTO_TEST_SUITE(TestCrestCache)

BOOST_AUTO_TEST_CASE(start_stop)
{
    crest::ConnectionPool conn_pool;
    crest::CacheOld cache(conn_pool);
}

BOOST_AUTO_TEST_CASE(test_sequential)
{
    crest::ConnectionPool conn_pool;
    crest::CacheOld cache(conn_pool);
    {
        auto a = cache.get_now("/market/10000002/orders/buy/?type=https://crest-tq.eveonline.com/inventory/types/34/");
        BOOST_CHECK_EQUAL(crest::CacheEntry::UPDATED, a.entry->status);
    }
    {
        auto a = cache.get_now("/market/10000002/orders/buy/?type=https://crest-tq.eveonline.com/inventory/types/34/");
        BOOST_CHECK_EQUAL(crest::CacheEntry::HIT, a.entry->status);
    }
    {
        auto a = cache.get_now("/market/10000002/orders/sell/?type=https://crest-tq.eveonline.com/inventory/types/34/");
        BOOST_CHECK_EQUAL(crest::CacheEntry::UPDATED, a.entry->status);
    }
    {
        auto a = cache.get_now("/market/10000002/orders/buy/?type=https://crest-tq.eveonline.com/inventory/types/34/");
        BOOST_CHECK_EQUAL(crest::CacheEntry::HIT, a.entry->status);
    }
}

BOOST_AUTO_TEST_CASE(test_parallel)
{
    crest::ConnectionPool conn_pool;
    crest::CacheOld cache(conn_pool);
    std::vector<crest::CacheOld::CacheLookupFutureResults> results;
    for (int i = 0; i < 100; ++i)
    {
        std::string type = "?type=https://crest-tq.eveonline.com/inventory/types/" + std::to_string(34 + i) + "/";
        results.push_back(cache.get_future("/market/10000002/orders/buy/" + type));
        results.push_back(cache.get_future("/market/10000002/orders/sell/" + type));
        results.push_back(cache.get_future("/market/10000002/orders/sell/" + type));
    }

    for (auto &result : results) result.wait();

}


BOOST_AUTO_TEST_SUITE_END()

