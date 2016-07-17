#include "Precompiled.hpp"
#include "CrestBulkMarketOrders.hpp"
#include "CrestCache.hpp"
#include <json/Reader.hpp>
#include <json/Writer.hpp>
#include "CrestJson.hpp"

void parse_crest_orders(
    std::vector<MarketOrder> &out,
    const std::vector<uint8_t> &crest_data)
{
    json::Parser parser(
        (const char*)crest_data.data(),
        (const char*)crest_data.data() + crest_data.size());

    read_crest_items(parser, &out);
}

void get_crest_bulk_market_orders(
    CrestCache &cache,
    std::vector<MarketOrderList> &order_lists)
{
    std::vector<CrestCacheEntry*> cache_entries;
    cache_entries.reserve(order_lists.size());

    //start cache lookups
    for (auto &i : order_lists)
    {
        std::stringstream ss;
        ss << "/market/" << i.region_id << "/orders/";
        ss << (i.buy ? "buy" : "sell");
        ss << "/?type=https://crest-tq.eveonline.com/inventory/types/" << i.type_id << "/";
        cache_entries.push_back(cache.get(ss.str()));
    }

    //get results
    for (size_t i = 0; i < order_lists.size(); ++i)
    {
        auto &out = order_lists[i];
        auto &entry = cache_entries[i];
        std::unique_lock<std::mutex> lock(entry->mutex);
        entry->wait(lock);
        if (entry->is_data_valid())
        {
            parse_crest_orders(out.orders, entry->data);
        }
    }
}
