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
        MarketOrderSlim() {}

        long long id;
        long long station_id;
        std::string range;
        std::string issued;
        bool buy;
        float price;
        int type;
        int duration;
        int min_volume;
        int volume;
        int volume_entered;
    };

    /**Read MarketOrder*/
    inline void read_json(json::Parser &parser, MarketOrderSlim *out)
    {
        typedef MarketOrderSlim T;
        static const auto reader = json::ObjectFieldReader<T, json::IgnoreUnknown>()
            .add<decltype(T::id), &T::id>("id")
            .add<decltype(T::station_id), &T::id>("stationID")
            .add<decltype(T::range), &T::range>("range")
            .add<decltype(T::issued), &T::issued>("issued")
            .add<decltype(T::buy), &T::buy>("buy")
            .add<decltype(T::price), &T::price>("price")
            .add<decltype(T::type), &T::type>("type")
            .add<decltype(T::duration), &T::duration>("duration")
            .add<decltype(T::min_volume), &T::min_volume>("min_volume")
            .add<decltype(T::volume), &T::volume>("volume")
            .add<decltype(T::volume_entered), &T::volume_entered>("volume_entered")
            ;
        reader.read(parser, out);
    }
}
