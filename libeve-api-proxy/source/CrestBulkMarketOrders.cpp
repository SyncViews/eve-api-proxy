#include "Precompiled.hpp"
#include "CrestBulkMarketOrders.hpp"
#include "CrestCache.hpp"
#include <json/Reader.hpp>
#include <json/Writer.hpp>
#include "CrestJson.hpp"

struct Location
{
    long long id;
    std::string name;
};
void read_json(json::Parser &parser, Location *out)
{
    static const auto reader = json::ObjectFieldReader<Location, json::IgnoreUnknown>()
        .add<decltype(Location::id), &Location::id>("id")
        .add<decltype(Location::name), &Location::name>("name");
    reader.read(parser, out);
}
void read_json(json::Parser &parser, MarketOrder *out)
{
    const auto location_f = [](json::Parser &parser, MarketOrder *out) -> void
    {
        Location tmp;
        read_json(parser, &tmp);
        out->station_id = tmp.id;
        out->station_name = tmp.name;
    };
    static const auto reader = json::ObjectFieldReader<MarketOrder, json::IgnoreUnknown>()
        .add<decltype(MarketOrder::id), &MarketOrder::id>("id")
        .add<decltype(MarketOrder::duration), &MarketOrder::duration>("duration")
        .add<decltype(MarketOrder::issued), &MarketOrder::issued>("issued")
        .add<decltype(MarketOrder::volume), &MarketOrder::volume>("volume")
        .add<decltype(MarketOrder::range), &MarketOrder::range>("range")
        .add<decltype(MarketOrder::price), &MarketOrder::price>("price")
        .add("location", location_f)
        ;
    Location location;
    reader.read(parser, out);
}
void write_json(json::Writer &writer, const MarketOrder &val)
{
    writer.start_obj();
    writer.prop("id", val.id);
    writer.prop("duration", val.duration);
    writer.prop("issued", val.issued);
    writer.prop("station_id", val.station_id);
    writer.prop("station_name", val.station_name);
    writer.prop("min_volume", val.min_volume);
    writer.prop("volume", val.volume);
    writer.prop("volume_entered", val.volume_entered);
    writer.prop("range", val.range);
    writer.prop("price", val.price);
    writer.end_obj();
}
void write_json(json::Writer &writer, const MarketOrderList &val)
{
    writer.start_obj();
    writer.prop("buy", val.buy);
    writer.prop("region", val.region_id);
    writer.prop("type", val.type_id);
    writer.prop("items", val.orders);
    writer.end_obj();
}

void parse_crest_orders(
    std::vector<MarketOrder> &out,
    const std::vector<uint8_t> &crest_data)
{
    json::Parser parser(
        (const char*)crest_data.data(),
        (const char*)crest_data.data() + crest_data.size());

    read_crest_items(parser, [&parser, &out]() -> void
    {
        read_json(parser, &out);
    });
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

std::string market_order_lists_json(const std::vector<MarketOrderList> &order_sets)
{
    return json::to_json(order_sets);
}
