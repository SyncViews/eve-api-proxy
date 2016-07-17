#include "Precompiled.hpp"
#include "Errors.hpp"
#include "BulkMarketHistory.hpp"
#include "CrestBulkMarketOrders.hpp"
#include "CrestCache.hpp"
#include "lib/Params.hpp"
#include "http/HttpStatusErrors.hpp"
#include <iostream>
#include <chrono>
#include <json/Reader.hpp>
#include <json/Writer.hpp>
#include <json/Copy.hpp>
#include "CrestJson.hpp"
#include "model/MarketHistoryDay.hpp"

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
            ss << "/market/" << j << "/history/?type=https://crest-tq.eveonline.com/inventory/types/" << i << "/";
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
                read_crest_items(parser, [&parser, &json_writer]() -> void
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

    http::HttpResponse resp;
    resp.status_code = 200;
    resp.body.assign(
        (const uint8_t*)json_writer.str().data(),
        (const uint8_t*)json_writer.str().data() + json_writer.str().size());
    return resp;
}

http::HttpResponse http_get_bulk_market_history_aggregated(CrestCache &cache, http::HttpRequest &request)
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
                auto day = read_crest_items<MarketHistoryDay>(cache_entry->data);
                
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

    http::HttpResponse resp;
    resp.status_code = 200;
    resp.body_str(json_writer.str());
    return resp;
}
