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

            explicit Region(int region_id)
                : region_id(region_id), entries(), fs(), expires(0), access_mutex()
            {}

            const int region_id;
            std::vector<Entry> entries;
            std::fstream fs;
            std::atomic<time_t> expires;
            std::mutex access_mutex;
        };
    public:
        /**Function to get all orders in a region. */
        typedef std::function<void(int region_id, GetMarketOrdersAllCb cb)>GetOrdersFunc;
        class RegionOrders
        {
        public:
            class iterator
            {
            public:
                struct value_type
                {
                    int type_id;
                    bool buy;
                    MarketOrdersSlim orders;
                };

                iterator& operator ++();
                value_type& operator *() { return value; }
                value_type* operator ->() { return &value; }
            private:
                typedef std::vector<Region::Entry>::iterator inner_iterator;
                Region &region;
                inner_iterator i;
                value_type value;

                iterator(Region &region, inner_iterator i);
                void load();

                friend class RegionOrders;
                friend bool operator != (const iterator &a, const iterator &b);
            };

            iterator begin();
            iterator end();
        private:
            friend class MarketOrdersCache;

            std::unique_lock<std::mutex> lock;
            Region &region;

            RegionOrders(Region &region);
        };

        MarketOrdersCache(GetOrdersFunc get_orders);
        ~MarketOrdersCache();

        void exit();

        /**Get orders from current cached data. */
        std::shared_ptr<const MarketOrdersSlim> get_orders(int region_id, int type_id, bool buy);
        RegionOrders get_orders(int region_id);
    private:
        GetOrdersFunc get_region_orders;
        std::unordered_map<int, Region> regions;

        /**Thread for update_thread_main.*/
        std::thread update_thread;
        std::atomic<bool> update_thread_exit;
        std::condition_variable update_thread_exit_cv;
        std::mutex update_thread_exit_cv_mutex;

        /**Creates initial map for regions with all entries but no data.*/
        static std::unordered_map<int, Region> create_regions_map();

        /**Entry point for thread responsible for keeping market data updated.*/
        void update_thread_main();

        /**Used by the update thread to update a region.*/
        void update_region(Region &region);
    };
}
