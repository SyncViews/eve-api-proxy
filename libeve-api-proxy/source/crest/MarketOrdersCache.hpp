#pragma once
#include <functional>
#include <mutex>
#include <future>
#include <fstream>
#include <ctime>
#include "MarketOrderSlim.hpp"
#include "MarketOrdersAll.hpp"


namespace crest
{
    typedef std::vector<MarketOrderSlim> MarketOrdersSlim;

    /**Data store for the cache of market orders.*/
    class MarketOrdersCache
    {
    public:
        /**Function to get all orders in a region. */
        
        typedef std::function<void(int region_id, GetMarketOrdersAllCb cb)>GetOrdersFunc;

        MarketOrdersCache(GetOrdersFunc get_orders);

        std::shared_ptr<const MarketOrdersSlim> get_type(int region_id, int type_id, bool buy);
        std::future<void> update_region_async(int region_id);
    private:
        /**A region a cached market orders. */
        struct Region
        {
            struct Entry
            {
                int id;
                unsigned start_byte;
                unsigned count;


                friend bool operator < (const Entry &a, const Entry &b)
                {
                    return a.id < b.id;
                }
            };

            Region(int region_id)
                : region_id(region_id), expires(0), access_mutex()
            {}
            Region(Region &&mv)
                : region_id(mv.region_id)
                , expires(mv.expires.load())
                , access_mutex()
            {}
            Region& operator = (Region &&mv)
            {
                region_id = mv.region_id;
                expires = mv.expires.load();
                return *this;
            }

            int region_id;
            std::vector<Entry> entries;
            std::fstream fs;
            std::atomic<time_t> expires;
            std::mutex access_mutex;
        };
     
        GetOrdersFunc get_orders;
        std::unordered_map<int, Region> regions;
        std::mutex mutex;
        std::atomic<int> updates_in_progress;
        std::condition_variable updates_in_progress_cvar;

        /**Updates a regions data if expired, must already hold region.access_lock.*/
        void update_region(int region_id);
    };
}
