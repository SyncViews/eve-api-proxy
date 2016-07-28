#pragma once
#include <functional>
#include <vector>
namespace crest
{
    class ConnectionPool;
    struct MarketOrderSlim;
    
    std::vector<MarketOrderSlim> get_market_orders_all(
        ConnectionPool &conn_pool,
        int region_id);
}
