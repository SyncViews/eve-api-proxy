#pragma once
#include <functional>
#include <mutex>
#include <ctime>
#include "MarketOrderSlim.hpp"


namespace crest
{
    typedef std::vector<MarketOrderSlim> MarketOrdersSlim;

    /**Data store for the cache of market orders.*/
    class MarketCacheStore
    {
    public:
        /**Function to get all orders in a region. */
        typedef std::function<MarketOrdersSlim(int region_id)> GetOrdersFunc;
        static const std::shared_ptr<const MarketOrdersSlim> EMPTY_ORDERS;

        MarketCacheStore(GetOrdersFunc get_orders) : get_orders(get_orders) {}

        /**Gets the market orders for a given type and region. */
        std::shared_ptr<const MarketOrdersSlim> get_type(int region_id, int type_id, bool buy)
        {
            auto region = get_region(region_id);

            auto type = region->types.find(type_id);
            if (type == region->types.end()) return EMPTY_ORDERS;

            return buy ? type->second.buy : type->second.sell;
        }
    private:
        /**A region a cached market orders. */
        struct Region
        {
        public:
            struct Type
            {
                std::shared_ptr<MarketOrdersSlim> buy;
                std::shared_ptr<MarketOrdersSlim> sell;
            };
            time_t expires;
            std::unordered_map<int, Type> types;
        };

        GetOrdersFunc get_orders;
        std::unordered_map<int, std::shared_ptr<Region>> regions;
        std::mutex mutex;

        std::shared_ptr<Region> get_region(int region_id);
    };
}
