#include "Precompiled.hpp"
#include "Errors.hpp"
#include "BulkMarketHistory.hpp"
#include "crest/CacheOld.hpp"
#include "crest/JsonHelpers.hpp"
#include "crest/Urls.hpp"
#include "lib/Params.hpp"
#include <iostream>
#include <chrono>
#include <json/Reader.hpp>
#include <json/Writer.hpp>
#include <json/Copy.hpp>
#include "model/MarketHistoryDay.hpp"

std::vector<crest::CacheEntry*> get_bulk_market_history(
    crest::CacheOld &cache, http::Request &request,
    const std::vector<int> &regions, const std::vector<int> &types)
{
    // Build requests
    size_t count = regions.size() * types.size();
    log_info() << "GET " << request.url.path << " with " << count << " histories" << std::endl;

    std::vector<crest::CacheEntry*> cache_entries;
    cache_entries.reserve(count);
    for (auto i : types)
    {
        for (auto j : regions)
        {
            cache_entries.push_back(cache.get(crest::market_history_url(j, i)));
        }
    }

    return cache_entries;
}

http::Response http_get_bulk_market_history(crest::CacheOld &cache, http::Request &request)
{
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<int> regions = params_regions(request);
    std::vector<int> types = params_inv_types(request);
    auto cache_entries = get_bulk_market_history(cache, request, regions, types);

    //Results
    json::Writer json_writer;
    json_writer.start_arr();
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
                //JsonReader doc(cache_entry->data);
                //auto items = doc.as_object().get_array("items").get_native();

                json_writer.start_obj();
                json_writer.prop("region", j);
                json_writer.prop("type", i);
                json_writer.end_obj();

                json::Parser parser(
                    (const char*)cache_entry->data.data(),
                    (const char*)cache_entry->data.data() + cache_entry->data.size());
                crest::read_crest_items(parser, [&parser, &json_writer]() -> void
                {
                    json::copy(json_writer, parser);
                });
            }
        }
    }
    json_writer.end_arr();
    auto end = std::chrono::high_resolution_clock::now();
    log_info() << "Cache lookup took "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
        << "ms." << std::endl;

    http::Response resp;
    resp.status_code(http::SC_OK);
    resp.body = json_writer.str();
    return resp;
}

http::Response http_get_bulk_market_history_aggregated(crest::CacheOld &cache, http::Request &request)
{
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<int> regions = params_regions(request);
    std::vector<int> types = params_inv_types(request);
    auto cache_entries = get_bulk_market_history(cache, request, regions, types);

    //Results
    json::Writer json_writer;
    json_writer.start_obj();
    size_t k = 0;
    for (auto i : types)
    {
        std::unordered_map<std::string, MarketHistoryDay> days;
        for (auto j : regions)
        {
            auto &cache_entry = cache_entries[k];
            std::unique_lock<std::mutex> lock(cache_entry->mutex);
            cache_entry->wait(lock);
            // Process
            if (cache_entry->is_data_valid())
            {
                auto region_days = crest::read_crest_items<std::vector<MarketHistoryDay>>(cache_entry->data);
                
                for (auto &day : region_days)
                {
                    auto ret = days.emplace(day.date, day);
                    if (!ret.second)
                    {
                        auto &aggregate = ret.first->second;
                        if (day.high_price > aggregate.high_price) aggregate.high_price = day.high_price;
                        if (day.low_price < aggregate.low_price) aggregate.low_price = day.low_price;
                        aggregate.order_count += day.order_count;
                        aggregate.volume += day.volume;
                        aggregate.avg_price += day.avg_price * day.volume;
                    }
                }
            }

            ++k;
        }

        if (!days.empty())
        {
            json_writer.key(std::to_string(i));
            json_writer.start_arr();

            for (auto &day : days)
            {
                day.second.avg_price /= day.second.volume; //avg_price was a sum in previous loop
                json_writer.value(day.second);
            }

            json_writer.end_arr();
        }
    }
    json_writer.end_obj();
    auto end = std::chrono::high_resolution_clock::now();
    log_info() << "Cache lookup took "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
        << "ms." << std::endl;

    http::Response resp;
    resp.status_code(http::SC_OK);
    resp.body = json_writer.str();
    return resp;
}
