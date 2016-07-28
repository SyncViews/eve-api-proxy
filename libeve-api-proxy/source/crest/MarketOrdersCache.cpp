#include "Precompiled.hpp"
#include "MarketOrdersCache.hpp"
#include <thread>
namespace crest
{
    const std::shared_ptr<const MarketOrdersSlim> MarketOrdersCache::EMPTY_ORDERS
        = std::make_shared<MarketOrdersSlim>();

    std::future<std::shared_ptr<const MarketOrdersSlim>>
    MarketOrdersCache::get_type_async(int region_id, int type_id, bool buy)
    {
        return std::async(std::launch::async,
            [this, region_id, type_id, buy]() -> std::shared_ptr<const MarketOrdersSlim>
        {
            return get_type(region_id, type_id, buy);
        });
    }

    std::shared_ptr<MarketOrdersCache::RegionData> MarketOrdersCache::get_region(int region_id)
    {
        std::unique_lock<std::mutex> lock(mutex);
        //try and get cached results
        auto region = regions.find(region_id);
        if (region != regions.end() && region->second.expires >= time(nullptr))
        {
            return region->second.data;
        }
        else
        {
            // Get cache entry
            if (region == regions.end())
            {
                region = regions.emplace(region_id, Region()).first;
            }
            auto &region_ref = region->second;
            lock.unlock();

            std::unique_lock<std::mutex> update_lock(region_ref.update_mutex);
            // Check expires again, may have blocked while another thread did the update
            if (region_ref.expires >= time(nullptr))
            {
                return region_ref.data;
            }
            
            // Get updated data
            auto orders = get_orders(region_id);

            auto new_data = std::make_shared<RegionData>();

            for (auto &order : orders)
            {
                auto &type = new_data->types[order.type];
                if (!type.buy) type.buy = std::make_shared<MarketOrdersSlim>();
                if (!type.sell) type.sell = std::make_shared<MarketOrdersSlim>();

                if (order.buy) type.buy->push_back(order);
                else type.sell->push_back(order);
            }

            //update cache under both locks
            lock.lock();
            region_ref.expires = time(nullptr) + 300;
            region_ref.data = new_data;
            return region_ref.data;
        }
    }
}
