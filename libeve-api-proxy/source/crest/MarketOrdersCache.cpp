#include "Precompiled.hpp"
#include "MarketOrdersCache.hpp"
#include "Error.hpp"
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

    MarketOrdersCache::MarketOrdersCache(GetOrdersFunc get_orders)
        : get_orders(get_orders), regions(), mutex(), updates_in_progress(0)
    {
        delete_dir_contents(CACHE_DIR);
    }

    std::shared_ptr<const MarketOrdersSlim> MarketOrdersCache::get_type(int region_id, int type_id, bool buy)
    {
        std::unique_lock<std::mutex> lock(mutex);
        //Get cache entry, and create a blank placeholder if needed
        auto region = regions.find(region_id);
        if (region == regions.end()) return EMPTY_ORDERS;
        auto &region_ref = region->second;

        lock.unlock();
        std::unique_lock<std::mutex> access_lock(region_ref.access_mutex);

        //Find entry
        auto id = type_id;
        if (buy) id = -id;
        auto entry = std::lower_bound(
            region_ref.entries.begin(),
            region_ref.entries.end(),
            id,
            [](const Region::Entry &entry, int id) { return entry.id < id; });
        if (entry == region_ref.entries.end() || entry->id != id)
            return EMPTY_ORDERS;
        //Read cache
        auto bytes = entry->count * sizeof(MarketOrderSlim);
        region_ref.fs.seekg(entry->start_byte);
        auto data = std::make_shared<MarketOrdersSlim>();
        data->resize(entry->count);
        region_ref.fs.read((char*)data->data(), bytes);
        if (region_ref.fs.gcount() != bytes) throw std::runtime_error("Failed to read cache file");
        //Done
        return data;
    }

    std::future<void> MarketOrdersCache::update_region_async(int region_id)
    {
        return std::async(std::launch::async,
            [this, region_id]() {
            update_region(region_id);
        });
    }

    void MarketOrdersCache::update_region(int region_id)
    {
        std::unique_lock<std::mutex> lock(mutex);
        //Get cache entry, and create a blank placeholder if needed
        auto &region = regions.emplace(region_id, Region(region_id)).first->second;
        //updates_in_progress_cvar.wait(lock,
        //    [this]() { return updates_in_progress < MAX_UPDATES_IN_PROGRESS; });
        //++updates_in_progress;
        lock.unlock();
        //Refresh cache
        std::unique_lock<std::mutex> access_lock(region.access_mutex);

        // Check if valid
        if (region.expires >= time(nullptr))
            return;

        //Get updated data
        try
        {
            std::unordered_map<int, std::vector<MarketOrderSlim>> files;
            get_orders(region.region_id, [&files](const MarketOrderSlim &order) -> void
            {
                int id = order.type;
                if (order.buy) id = -id;

                auto &file = files[id];
                file.push_back(order);
            });

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
            if (!region.fs.good()) throw std::runtime_error("Failed to write cache file");

            region.expires = time(nullptr) + 300;
            //if (--updates_in_progress <= 0) updates_in_progress_cvar.notify_one();
        }
        catch (const std::exception &e)
        {
            log_error() << "Failed to update orders cache for " << region_id << ": " << e.what() << std::endl;
            throw;
        }
    }
}
