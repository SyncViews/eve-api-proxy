#include <boost/test/unit_test.hpp>
#include <iostream>
#include "CrestCache.hpp"
#include "CrestBulkMarketOrders.hpp"

BOOST_AUTO_TEST_SUITE(TestCrestBulkMarketOrders)

BOOST_AUTO_TEST_CASE(test)
{
    CrestCache cache;
    std::vector<MarketOrderList> orders;
    orders.emplace_back(true, 10000002, 34);
    orders.emplace_back(false, 10000002, 34);

    get_crest_bulk_market_orders(cache, orders);

    BOOST_CHECK(!orders[0].orders.empty());
    BOOST_CHECK(!orders[1].orders.empty());

    std::string json = json::to_json(orders);
    std::cout << "market_order_lists_json" << std::endl;
    std::cout.write(json.data(), std::min((size_t)100, json.size()));
    std::cout << std::endl << std::endl;
}






BOOST_AUTO_TEST_SUITE_END()

