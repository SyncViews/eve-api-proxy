#pragma once
#include <functional>
#include <mutex>
#include <ctime>
#include "MarketOrderSlim.hpp"


namespace crest
{
    typedef std::vector<MarketOrderSlim> MarketOrdersSlim;
    typedef std::function<MarketOrdersSlim(int region_id)> GetOrdersFunc;
    struct MarketCacheRegion
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

    class MarketCache
    {
    public:
        static const std::shared_ptr<const MarketOrdersSlim> EMPTY_ORDERS;

        MarketCache(GetOrdersFunc get_orders) : get_orders(get_orders) {}

        std::shared_ptr<const MarketOrdersSlim> get_type(int region_id, int type_id, bool buy)
        {
            auto region = get_region(region_id);

            auto type = region->types.find(type_id);
            if (type == region->types.end()) return EMPTY_ORDERS;

            return buy ? type->second.buy : type->second.sell;
        }
        std::shared_ptr<const MarketCacheRegion> get_region(int region_id)
        {
            return do_get_region(region_id);
        }
    private:
        GetOrdersFunc get_orders;
        std::unordered_map<int, std::shared_ptr<MarketCacheRegion>> regions;
        std::mutex mutex;

        std::shared_ptr<MarketCacheRegion> do_get_region(int region_id);
    };
}
