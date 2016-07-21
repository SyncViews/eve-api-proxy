#include "Precompiled.hpp"
#include "MarketOrdersCache.hpp"
namespace crest
{
    const std::shared_ptr<const MarketOrdersSlim> MarketCacheStore::EMPTY_ORDERS
        = std::make_shared<MarketOrdersSlim>();

    std::shared_ptr<MarketCacheStore::Region> MarketCacheStore::get_region(int region_id)
    {
        std::unique_lock<std::mutex> lock(mutex);
        auto region = regions.find(region_id);
        if (region != regions.end() && region->second->expires >= time(nullptr))
        {
            return region->second;
        }
        else
        {
            auto orders = get_orders(region_id);

            auto new_region = std::make_shared<Region>();
            new_region->expires = time(nullptr) + 300;

            for (auto &order : orders)
            {
                auto &type = new_region->types[order.type];
                if (!type.buy) type.buy = std::make_shared<MarketOrdersSlim>();
                if (!type.sell) type.sell = std::make_shared<MarketOrdersSlim>();

                if (order.buy) type.buy->push_back(order);
                else type.sell->push_back(order);
            }

            regions[region_id] = new_region;
            return new_region;
        }
    }
}
