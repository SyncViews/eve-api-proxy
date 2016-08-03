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
        void exit();
        
        std::shared_ptr<const MarketOrdersSlim> get_orders(int region_id, int type_id, bool buy)
        {
            return market_orders.get_orders(region_id, type_id, buy);
        }

        /**Needed currently for CacheOld.*/
        ConnectionPool &get_conn_pool() { return conn_pool; }
    private:
        ConnectionPool conn_pool;
        MarketOrdersCache market_orders;
    };
}
