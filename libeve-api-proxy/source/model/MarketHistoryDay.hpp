#pragma once
#include <string>
#include <json/Reader.hpp>
#include <json/Writer.hpp>

struct MarketHistoryDay
{
    std::string date;
    double avg_price;
    double high_price;
    double low_price;
    int order_count;
    int64_t volume;
};
inline void write_json(json::Writer &writer, const MarketHistoryDay &day)
{
    writer.start_obj();
    writer.prop("date", day.date);
    writer.prop("avg_price", day.avg_price);
    writer.prop("high_price", day.high_price);
    writer.prop("low_price", day.low_price);
    writer.prop("order_count", day.order_count);
    writer.prop("volume", day.volume);
    writer.end_obj();
}
inline void read_json(json::Parser &parser, MarketHistoryDay *day)
{
    static const auto reader = json::ObjectFieldReader<MarketHistoryDay, json::IgnoreUnknown>()
        .add<decltype(MarketHistoryDay::date), &MarketHistoryDay::date>("date")
        .add<decltype(MarketHistoryDay::high_price), &MarketHistoryDay::high_price>("highPrice")
        .add<decltype(MarketHistoryDay::low_price), &MarketHistoryDay::low_price>("lowPrice")
        .add<decltype(MarketHistoryDay::order_count), &MarketHistoryDay::order_count>("orderCount")
        .add<decltype(MarketHistoryDay::volume), &MarketHistoryDay::volume>("volume")
        .add<decltype(MarketHistoryDay::avg_price), &MarketHistoryDay::avg_price>("avgPrice");
    reader.read(parser, day);
}
