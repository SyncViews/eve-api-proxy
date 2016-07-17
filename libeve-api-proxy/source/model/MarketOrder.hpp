#pragma once
#include <string>
#include <vector>
#include <json/Reader.hpp>
#include <json/Writer.hpp>

/**@brief A market order. */
struct MarketOrder
{
    MarketOrder() {}

    long long id;
    int duration;
    std::string issued;
    long long station_id;
    std::string station_name;
    int min_volume;
    int volume;
    int volume_entered;
    std::string range;
    float price;
};
/**A list of MarketOrder for either buy or sell, and a single type and region.*/
struct MarketOrderList
{
    MarketOrderList(bool buy, int region_id, int type_id)
        : buy(buy), region_id(region_id), type_id(type_id), orders()
    {}

    bool buy;
    int region_id;
    int type_id;
    std::vector<MarketOrder> orders;
};

/**Read MarketOrder*/
inline void read_json(json::Parser &parser, MarketOrder *out)
{
    const auto location_f = [](json::Parser &parser, MarketOrder *out) -> void
    {
        static const auto reader = json::ObjectFieldReader<MarketOrder, json::IgnoreUnknown>()
            .add<decltype(MarketOrder::station_id), &MarketOrder::station_id>("id")
            .add<decltype(MarketOrder::station_name), &MarketOrder::station_name>("name");
        reader.read(parser, out);
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
    reader.read(parser, out);
}

/**Write MarketOrder*/
inline void write_json(json::Writer &writer, const MarketOrder &val)
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
/**Write MarketOrderList.*/
inline void write_json(json::Writer &writer, const MarketOrderList &val)
{
    writer.start_obj();
    writer.prop("buy", val.buy);
    writer.prop("region", val.region_id);
    writer.prop("type", val.type_id);
    writer.prop("items", val.orders);
    writer.end_obj();
}
