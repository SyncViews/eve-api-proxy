#pragma once
#include <string>
#include <vector>
#include <json/Reader.hpp>
#include <json/Writer.hpp>

namespace crest
{
    /**@brief A market order. New structure matching /orders/all/*/
    struct MarketOrderSlim
    {
        static const int RANGE_STATION = -1;
        static const int RANGE_REGION = -2;
        MarketOrderSlim() {}

        long long id;
        long long station_id;
        int range;
        time_t issued;
        bool buy;
        float price;
        int type;
        int duration;
        int min_volume;
        int volume;
        int volume_entered;
    };

    inline void read_market_order_slim_range(json::Parser &parser, int *range)
    {
        auto str = json::read_json<std::string>(parser);
        if (str == "station") *range = MarketOrderSlim::RANGE_STATION;
        else if (str == "region") *range = MarketOrderSlim::RANGE_REGION;
        else *range = std::stoi(str);
    }

    /**Read MarketOrder*/
    inline void read_json(json::Parser &parser, MarketOrderSlim *out)
    {
        typedef MarketOrderSlim T;
        static const auto reader = json::ObjectFieldReader<T, json::IgnoreUnknown>()
            .add<decltype(T::id), &T::id>("id")
            .add<decltype(T::station_id), &T::station_id>("stationID")
            .add<decltype(T::range), &T::range, read_market_order_slim_range>("range")
            .add<decltype(T::issued), &T::issued, json::read_json_time>("issued")
            .add<decltype(T::buy), &T::buy>("buy")
            .add<decltype(T::price), &T::price>("price")
            .add<decltype(T::type), &T::type>("type")
            .add<decltype(T::duration), &T::duration>("duration")
            .add<decltype(T::min_volume), &T::min_volume>("minVolume")
            .add<decltype(T::volume), &T::volume>("volume")
            .add<decltype(T::volume_entered), &T::volume_entered>("volumeEntered")
            ;
        reader.read(parser, out);
    }
    inline void write_json(json::Writer &writer, const MarketOrderSlim &order)
    {
        writer.start_obj();
        writer.prop("id", order.id);
        writer.prop("station_id", order.station_id);
        if (order.range == MarketOrderSlim::RANGE_REGION)
            writer.prop("range", "region");
        else if (order.range == MarketOrderSlim::RANGE_STATION)
            writer.prop("range", "station");
        else writer.prop("range", std::to_string(order.range));
        writer.prop("issued", json::time_to_iso_str(order.issued));
        writer.prop("buy", order.buy);
        writer.prop("price", order.price);
        writer.prop("type", order.type);
        writer.prop("duration", order.duration);
        writer.prop("volume", order.volume);
        writer.prop("min_volume", order.min_volume);
        writer.prop("volume_entered", order.volume_entered);
        writer.end_obj();
    }
}
