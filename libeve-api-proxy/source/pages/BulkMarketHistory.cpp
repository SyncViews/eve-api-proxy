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
http::HttpResponse http_get_bulk_market_history(CrestCache &cache, http::HttpRequest &request)
{
    // Get params
    std::vector<int> regions = params_regions(request);
    std::vector<int> types = params_inv_types(request);

    // Build requests
    size_t count = regions.size() * types.size();
    log_info() << "GET /bulk-market-history with " << count << " histories" << std::endl;

    // Start cache lookups
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<CrestCacheEntry*> cache_entries;
    cache_entries.reserve(count);
    for (auto i : regions)
    {
        for (auto j : types)
        {
            std::stringstream ss;
            ss << "/market/" << i << "/types/" << j << "/history/";
            cache_entries.push_back(cache.get(ss.str()));
        }
    }
    //Results
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> json(buffer);
    json.StartArray();
    size_t k = 0;
    for (auto i : regions)
    {
        for (auto j : types)
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
                json.Key("region"); json.Int(i);
                json.Key("type");   json.Int(j);
                json.Key("items");
                items->Accept(json);

                json.EndObject();
            }

            ++k;
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
