#pragma once
#include <functional>
#include <mutex>
#include <future>
#include <ctime>
#include "MarketOrderSlim.hpp"


namespace crest
{
    typedef std::vector<MarketOrderSlim> MarketOrdersSlim;

    /**Data store for the cache of market orders.*/
    class MarketOrdersCache
    {
    public:
        /**Function to get all orders in a region. */
        typedef std::function<MarketOrdersSlim(int region_id)> GetOrdersFunc;
        static const std::shared_ptr<const MarketOrdersSlim> EMPTY_ORDERS;

        MarketOrdersCache(GetOrdersFunc get_orders) : get_orders(get_orders) {}

        /**Gets the market orders for a given type and region. */
        std::shared_ptr<const MarketOrdersSlim> get_type(int region_id, int type_id, bool buy)
        {
            auto region = get_region(region_id);

            auto type = region->types.find(type_id);
            if (type == region->types.end()) return EMPTY_ORDERS;

            return buy ? type->second.buy : type->second.sell;
        }

        std::future<std::shared_ptr<const MarketOrdersSlim>> get_type_async(int region_id, int type_id, bool buy);
    private:
        struct RegionData
        {
            struct Type
            {
                std::shared_ptr<MarketOrdersSlim> buy;
                std::shared_ptr<MarketOrdersSlim> sell;
            };
            std::unordered_map<int, Type> types;
        };
        /**A region a cached market orders. */
        struct Region
        {
            Region() : data(), expires(0), update_mutex() {}
            Region(Region &&mv)
                : data(mv.data)
                , expires(mv.expires.load())
                , update_mutex()
            {}
            Region& operator = (Region &&mv)
            {
                data = mv.data;
                expires = mv.expires.load();
                return *this;
            }

            std::shared_ptr<RegionData> data;
            std::atomic<time_t> expires;
            std::mutex update_mutex;
        };

        GetOrdersFunc get_orders;
        std::unordered_map<int, Region> regions;
        std::mutex mutex;

        std::shared_ptr<RegionData> get_region(int region_id);
    };
}
