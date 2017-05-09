#include "Precompiled.hpp"
#include "MarketOrdersCache.hpp"
#include "MarketSummaryCsv.hpp"
#include "Error.hpp"
#include "model/EveRegions.hpp"
#include <thread>
#include <boost/filesystem.hpp>
namespace crest
{
    namespace
    {
        namespace fs = boost::filesystem;
        const auto EMPTY_ORDERS = std::make_shared<const MarketOrdersSlim>();
        static const int MAX_UPDATES_IN_PROGRESS = 50;
        const std::string CACHE_DIR = "cache/market_orders/";

        void delete_dir_contents(const std::string &dir)
        {
            fs::path path(dir);
            if (fs::exists(path))
            {
                for (fs::directory_iterator i(path), end; i != end; ++i)
                {
                    fs::remove_all(i->path());
                }
            }
        }
        std::string get_region_cache_path(int region_id)
        {
            return CACHE_DIR + std::to_string(region_id);
        }
    }

    std::unordered_map<int, MarketOrdersCache::Region> MarketOrdersCache::create_regions_map()
    {
        std::unordered_map<int, Region> ret;
        for (auto id : EVE_REGION_IDS)
        {
            ret.emplace(std::piecewise_construct, std::make_tuple(id), std::make_tuple(id));
        }
        return ret;
    }
    MarketOrdersCache::MarketOrdersCache(GetOrdersFunc get_region_orders)
        : get_region_orders(get_region_orders), regions(create_regions_map())
        , update_thread(), update_thread_exit(false), update_thread_exit_cv(), update_thread_exit_cv_mutex()
    {
        delete_dir_contents(CACHE_DIR);
        update_thread = std::thread(std::bind(&MarketOrdersCache::update_thread_main, this));
    }

    MarketOrdersCache::~MarketOrdersCache()
    {
        exit();
    }

    void MarketOrdersCache::exit()
    {
        if (update_thread.joinable())
        {
            update_thread_exit = true;
            update_thread_exit_cv.notify_all();
            update_thread.join();
        }
    }

    std::shared_ptr<const MarketOrdersSlim> MarketOrdersCache::get_orders(int region_id, int type_id, bool buy)
    {
        auto it = regions.find(region_id);
        if (it == regions.end()) throw std::runtime_error("Unknown region " + std::to_string(region_id));
        auto &region = it->second;

        std::unique_lock<std::mutex> lock(region.access_mutex);
        //Find entry
        auto id = type_id;
        if (buy) id = -id;
        auto entry = std::lower_bound(
            region.entries.begin(),
            region.entries.end(),
            id,
            [](const Region::Entry &entry, int id) { return entry.id < id; });
        if (entry == region.entries.end() || entry->id != id)
            return EMPTY_ORDERS;
        //Read cache
        auto bytes = entry->count * sizeof(MarketOrderSlim);
        region.fs.seekg(entry->start_byte);
        auto data = std::make_shared<MarketOrdersSlim>();
        data->resize(entry->count);
        region.fs.read((char*)data->data(), bytes);
        if (region.fs.gcount() != bytes) throw std::runtime_error("Failed to read cache file");
        //Done
        return data;
    }

    MarketOrdersCache::RegionOrders MarketOrdersCache::get_orders(int region_id)
    {
        auto it = regions.find(region_id);
        if (it == regions.end()) throw std::runtime_error("Unknown region " + std::to_string(region_id));
        return RegionOrders(it->second);
    }

    void MarketOrdersCache::update_thread_main()
    {
        set_thread_name("MarketOrdersCache update");
        auto update_next = regions.begin();
        while (true)
        {
            if (update_next == regions.end())
            {
                update_next = regions.begin();
                auto now = time(nullptr);
                if (now < update_next->second.expires)
                {
                    //update summary csv after completed update of all regions, and have spare time
                    update_market_summary_csv(*this);
                }
            }
            auto &region = update_next->second;

            auto now = time(nullptr);
            if (now < region.expires)
            {
                log_debug() << "Waiting " << (region.expires - now) << " seconds until next region update" << std::endl;
                std::this_thread::sleep_until(std::chrono::system_clock::from_time_t(region.expires));
                assert(time(nullptr) >= region.expires);
            }

            update_region(region);

            ++update_next;
        }
    }

    void MarketOrdersCache::update_region(Region &region)
    {
        assert(region.expires <= time(nullptr));
        //Get updated data
        try
        {
            std::unordered_map<int, std::vector<MarketOrderSlim>> files;
            get_region_orders(region.region_id, [&files](const MarketOrderSlim &order) -> void
            {
                int id = order.type;
                if (order.buy) id = -id;

                auto &file = files[id];
                file.push_back(order);
            });

            //Got all data ready, just hold lock while updating Region and the filesystem
            std::unique_lock<std::mutex> lock(region.access_mutex);
            region.entries.clear();
            region.entries.reserve(files.size());

            auto path = get_region_cache_path(region.region_id);
            fs::create_directories(CACHE_DIR);
            region.fs.close();
            region.fs.open(path, std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
            if (!region.fs.good()) throw std::runtime_error("Failed to open cache file");
            for (auto &type : files)
            {
                Region::Entry entry = {
                    type.first,
                    (unsigned)region.fs.tellp(),
                    (unsigned)type.second.size()
                };
                region.entries.push_back(entry);
                region.fs.write((const char*)type.second.data(), sizeof(MarketOrderSlim)*type.second.size());
            }
            std::sort(region.entries.begin(), region.entries.end());
            region.fs.flush();
            if (!region.fs.good()) throw std::runtime_error("Failed to write cache file");

            region.expires = time(nullptr) + 300;
        }
        catch (const std::exception &e)
        {
            log_error() << "Failed to update orders cache for " << region.region_id << ": " << e.what() << std::endl;
            region.expires = time(nullptr) + 60;
        }
    }


    MarketOrdersCache::RegionOrders::iterator MarketOrdersCache::RegionOrders::begin()
    {
        return iterator(region, region.entries.begin());
    }

    MarketOrdersCache::RegionOrders::iterator MarketOrdersCache::RegionOrders::end()
    {
        return iterator(region, region.entries.end());
    }

    MarketOrdersCache::RegionOrders::RegionOrders(Region &region)
        : lock(region.access_mutex), region(region)
    {
    }
    MarketOrdersCache::RegionOrders::iterator & MarketOrdersCache::RegionOrders::iterator::operator++()
    {
        ++i;
        load();
        return *this;
    }
    MarketOrdersCache::RegionOrders::iterator::iterator(Region &region, inner_iterator i)
        : region(region), i(i), value()
    {
        load();
    }
    void MarketOrdersCache::RegionOrders::iterator::load()
    {
        value.orders.clear();
        if (i != region.entries.end())
        {
            value.buy = i->id < 0;
            value.type_id = std::abs(i->id);
            //Read cache
            auto bytes = i->count * sizeof(MarketOrderSlim);
            region.fs.seekg(i->start_byte);
            value.orders.resize(i->count);
            region.fs.read((char*)value.orders.data(), bytes);
            if (region.fs.gcount() != bytes) throw std::runtime_error("Failed to read cache file");
        }
    }
    bool operator!=(const MarketOrdersCache::RegionOrders::iterator &a, const MarketOrdersCache::RegionOrders::iterator &b)
    {
        return a.i != b.i;
    }
}
