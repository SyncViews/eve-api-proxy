#include "Precompiled.hpp"
#include "CrestBulkMarketOrders.hpp"
#include "CrestCache.hpp"
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

const char *do_parse_crest_orders(
    std::vector<MarketOrder> &out,
    const std::vector<uint8_t> &crest_data)
{
    rapidjson::Document doc;
    doc.Parse((const char*)crest_data.data(), crest_data.size());
    if (!doc.IsObject()) return "Expected object root";
    
    auto items_it = doc.FindMember("items");
    if (items_it == doc.MemberEnd()) return "Failed to items member";
    auto &items = items_it->value;
    if (!items.IsArray()) return "Expected items to be an array";
    out.reserve(items.Size());
    for (auto &item : items.GetArray())
    {
        if (!item.IsObject()) return "Items should be objects";
        out.emplace_back();
        auto &order = out.back();

        auto it = item.FindMember("id");
        if (it == item.MemberEnd() || !it->value.IsInt64()) return "Invalid item id";
        order.id = it->value.GetInt64();

        it = item.FindMember("duration");
        if (it == item.MemberEnd() || !it->value.IsInt()) return "Invalid item duration";
        order.duration = it->value.GetInt();

        it = item.FindMember("issued");
        if (it == item.MemberEnd() || !it->value.IsString()) return "Invalid item issued";
        order.issued.assign(it->value.GetString(), it->value.GetStringLength());

        it = item.FindMember("minVolume");
        if (it == item.MemberEnd() || !it->value.IsInt()) return "Invalid item minVolume";
        order.min_volume = it->value.GetInt();

        it = item.FindMember("volume");
        if (it == item.MemberEnd() || !it->value.IsInt()) return "Invalid item volume";
        order.volume = it->value.GetInt();

        it = item.FindMember("volumeEntered");
        if (it == item.MemberEnd() || !it->value.IsInt()) return "Invalid item volumeEntered";
        order.volume_entered = it->value.GetInt();

        it = item.FindMember("range");
        if (it == item.MemberEnd() || !it->value.IsString()) return "Invalid item range";
        order.range.assign(it->value.GetString(), it->value.GetStringLength());

        it = item.FindMember("price");
        if (it == item.MemberEnd() || !it->value.IsFloat()) return "Invalid item price";
        order.price = it->value.GetFloat();


        it = item.FindMember("location");
        if (it == item.MemberEnd() || !it->value.IsObject()) return "Invalid item location";

        auto it2 = it->value.FindMember("id");
        if (it2 == it->value.MemberEnd() || !it2->value.IsInt()) return "Invalid item location id";
        order.station_id = it2->value.GetInt();

        it2 = it->value.FindMember("name");
        if (it2 == it->value.MemberEnd() || !it2->value.IsString()) return "Invalid item location name";
        order.station_name.assign(it2->value.GetString(), it2->value.GetStringLength());
    }

    return nullptr;
}

void parse_crest_orders(
    std::vector<MarketOrder> &out,
    const std::vector<uint8_t> &crest_data)
{
    auto err = do_parse_crest_orders(out, crest_data);
    if (err) throw std::runtime_error(err);
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
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    
    writer.StartArray();
    for (auto &i : order_sets)
    {
        writer.StartObject();
        writer.Key("buy"); writer.Bool(i.buy);
        writer.Key("region"); writer.Int(i.region_id);
        writer.Key("type"); writer.Int(i.type_id);
        
        writer.Key("items");
        writer.StartArray();
        for (auto &j : i.orders)
        {
            writer.StartObject();
            writer.Key("id");               writer.Int64(j.id);
            writer.Key("duration");         writer.Int(j.duration);
            writer.Key("issued");           writer.String(j.issued.data(), (unsigned)j.issued.size());
            writer.Key("station_id");       writer.Int(j.station_id);
            writer.Key("station_name");     writer.String(j.station_name.data(), (unsigned)j.station_name.size());
            writer.Key("min_volume");       writer.Int(j.min_volume);
            writer.Key("volume");           writer.Int(j.volume);
            writer.Key("volume_entered");   writer.Int(j.volume_entered);
            writer.Key("range");            writer.String(j.range.data(), (unsigned)j.range.size());
            writer.Key("price");            writer.Double(j.price);
            writer.EndObject();
        }
        writer.EndArray();

        writer.EndObject();
    }
    writer.EndArray();
    return std::string(buffer.GetString(), buffer.GetSize());
}
