#include "Precompiled.hpp"
#include "Errors.hpp"
#include "BulkMarketHistory.hpp"
#include "CrestBulkMarketOrders.hpp"
#include "CrestCache.hpp"
#include "lib/Params.hpp"
#include "http/HttpStatusErrors.hpp"
#include <iostream>
#include <chrono>
#include "json/JsonReader.hpp"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

struct MarketHistoryDay
{
    std::string date;
    double avg_price;
    double high_price;
    double low_price;
    int order_count;
    int64_t volume;
};

template<class T>
void market_history_day_to_json(rapidjson::Writer<T> &writer, const MarketHistoryDay &day)
{
    writer.StartObject();
    writer.Key("date");         writer.String(day.date);
    writer.Key("avg_price");    writer.Double(day.avg_price);
    writer.Key("high_price");   writer.Double(day.high_price);
    writer.Key("low_price");    writer.Double(day.low_price);
    writer.Key("order_count");  writer.Int(day.order_count);
    writer.Key("volume");       writer.Int64(day.volume);
    writer.EndObject();
}

std::vector<CrestCacheEntry*> get_bulk_market_history(
    CrestCache &cache, http::HttpRequest &request,
    const std::vector<int> &regions, const std::vector<int> &types)
{
    // Build requests
    size_t count = regions.size() * types.size();
    log_info() << "GET /" << request.url.path << " with " << count << " histories" << std::endl;

    std::vector<CrestCacheEntry*> cache_entries;
    cache_entries.reserve(count);
    for (auto i : types)
    {
        for (auto j : regions)
        {
            std::stringstream ss;
            ss << "/market/" << j << "/types/" << i << "/history/";
            cache_entries.push_back(cache.get(ss.str()));
        }
    }

    return cache_entries;
}

http::HttpResponse http_get_bulk_market_history(CrestCache &cache, http::HttpRequest &request)
{
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<int> regions = params_regions(request);
    std::vector<int> types = params_inv_types(request);
    auto cache_entries = get_bulk_market_history(cache, request, regions, types);

    //Results
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> json(buffer);
    json.StartArray();
    size_t k = 0;
    for (auto i : types)
    {
        for (auto j : regions)
        {
            auto &cache_entry = cache_entries[k];
            std::unique_lock<std::mutex> lock(cache_entry->mutex);
            cache_entry->wait(lock);
            // Process
            if (cache_entry->is_data_valid())
            {
                JsonReader doc(cache_entry->data);
                auto items = doc.as_object().get_array("items").get_native();

                json.StartObject();
                json.Key("region"); json.Int(j);
                json.Key("type");   json.Int(i);
                json.Key("items");
                items->Accept(json);

                json.EndObject();
            }
        }
    }
    json.EndArray();
    auto end = std::chrono::high_resolution_clock::now();
    log_info() << "Cache lookup took "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
        << "ms." << std::endl;

    http::HttpResponse resp;
    resp.status_code = 200;
    resp.body.assign(
        (const uint8_t*)buffer.GetString(),
        (const uint8_t*)buffer.GetString() + buffer.GetSize());
    return resp;
}

http::HttpResponse http_get_bulk_market_history_aggregated(CrestCache &cache, http::HttpRequest &request)
{
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<int> regions = params_regions(request);
    std::vector<int> types = params_inv_types(request);
    auto cache_entries = get_bulk_market_history(cache, request, regions, types);

    //Results
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> json(buffer);
    json.StartObject();
    size_t k = 0;
    for (auto i : types)
    {
        std::map<std::string, MarketHistoryDay> days;
        for (auto j : regions)
        {
            auto &cache_entry = cache_entries[k];
            std::unique_lock<std::mutex> lock(cache_entry->mutex);
            cache_entry->wait(lock);
            // Process
            if (cache_entry->is_data_valid())
            {
                JsonReader doc(cache_entry->data);
                auto items = doc.as_object().get_array("items");
                for (auto k = 0; k < items.size(); ++k)
                {
                    auto jday = items.get_object(k);
                    auto date_str = jday.get_str("date");
                    auto &day = days[date_str];
                    if (day.date.empty())
                    {
                        day.date = date_str;
                        day.high_price = jday.get_float("highPrice");
                        day.low_price = jday.get_float("lowPrice");
                        day.order_count = jday.get_int32("orderCount");
                        day.volume = jday.get_int64("volume");
                        day.avg_price = jday.get_float("avgPrice") * day.volume;
                    }
                    else
                    {
                        auto high = jday.get_float("highPrice");
                        auto low = jday.get_float("lowPrice");
                        auto volume = jday.get_int64("volume");
                        if (high > day.high_price) day.high_price = high;
                        if (low  < day.low_price) day.low_price = low;
                        day.order_count += jday.get_int32("orderCount");
                        day.volume += volume;
                        day.avg_price += jday.get_float("avgPrice") * volume;
                    }
                }
            }

            ++k;
        }

        if (!days.empty())
        {
            auto key = std::to_string(i);
            json.Key(key.data(), key.size(), true);
            json.StartArray();

            for (auto &day : days)
            {
                day.second.avg_price /= day.second.volume; //avg_price was a sum in previous loop
                market_history_day_to_json(json, day.second);
            }

            json.EndArray();
        }
    }
    json.EndObject();
    auto end = std::chrono::high_resolution_clock::now();
    log_info() << "Cache lookup took "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
        << "ms." << std::endl;

    http::HttpResponse resp;
    resp.status_code = 200;
    resp.body.assign(
        (const uint8_t*)buffer.GetString(),
        (const uint8_t*)buffer.GetString() + buffer.GetSize());
    return resp;
}
