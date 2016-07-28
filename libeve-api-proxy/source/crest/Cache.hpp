#pragma once
#include "MarketOrdersCache.hpp"
#include "ConnectionPool.hpp"

namespace crest
{
    class Cache
    {
    public:
        Cache();
        ~Cache();

        std::shared_ptr<const MarketOrdersSlim> get_orders(int region_id, int type_id, bool buy)
        {
            return market_orders.get_type(region_id, type_id, buy);
        }
        std::future<std::shared_ptr<const MarketOrdersSlim>> get_orders_async(int region_id, int type_id, bool buy)
        {
            return market_orders.get_type_async(region_id, type_id, buy);
        }
    private:
        ConnectionPool conn_pool;
        MarketOrdersCache market_orders;

        MarketOrdersSlim get_region_orders(int region_id);
    };
}
