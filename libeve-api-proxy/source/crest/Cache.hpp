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
        std::future<void> update_region_orders_async(int region_id)
        {
            return market_orders.update_region_async(region_id);
        }
        void update_region_orders(const int *regions, size_t count)
        {
            std::unique_ptr<std::future<void>[]> futures(new std::future<void>[count]);
            for (size_t i = 0; i < count; ++i) futures[i] = update_region_orders_async(regions[i]);
            for (size_t i = 0; i < count; ++i) futures[i].wait();
        }
        void update_region_orders(const int region_id)
        {
            update_region_orders_async(region_id).wait();
        }
        void update_region_orders(const std::vector<int> &regions)
        {
            return update_region_orders(regions.data(), regions.size());
        }
        /**Needed currently for CacheOld.*/
        ConnectionPool &get_conn_pool() { return conn_pool; }
    private:
        ConnectionPool conn_pool;
        MarketOrdersCache market_orders;
    };
}
