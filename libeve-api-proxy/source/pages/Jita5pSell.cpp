#include "Precompiled.hpp"
#include "Errors.hpp"
#include "Jita5pSell.hpp"
#include "CrestBulkMarketOrders.hpp"
#include "EveRegions.hpp"
#include <iostream>
#include <chrono>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
http::HttpResponse http_get_jita_5p_sell(CrestCache &cache, http::HttpRequest &request)
{
    // Get params
    std::vector<MarketOrderList> order_sets;
    for (auto &i : request.url.query_array_param("type"))
    {
        auto type = std::stoi(i);
        order_sets.emplace_back(false, EVE_THE_FORGE_ID, type);
    }
    if (order_sets.empty()) return http_bad_request(request, "At least one type required.");

    // Build requests
    log_info() << "GET /jita-5p-sell with " << order_sets.size() << " types" << std::endl;
    
    // Make requests
    auto start = std::chrono::high_resolution_clock::now();
    get_crest_bulk_market_orders(cache, order_sets);
    auto end = std::chrono::high_resolution_clock::now();
    log_info() << "Cache lookup took "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
        << "ms." << std::endl;
    
    // Response
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> json(buffer);
    json.StartObject();
    for (auto &i : order_sets)
    {
        std::sort(i.orders.begin(), i.orders.end(),
            [](const MarketOrder &a, const MarketOrder &b) {
                return a.price < b.price;
            });
        int64_t total_volume = 0;
        for (auto &a : i.orders) total_volume += a.volume;
        auto avg_volume = (total_volume + 20 - 1) / 20;
        auto remaining_volume = avg_volume;
        double total_price = 0;
        for (auto &a : i.orders)
        {
            if (remaining_volume > a.volume)
            {
                remaining_volume -= a.volume;
                total_price += a.price * a.volume;
            }
            else
            {
                total_price += a.price * remaining_volume;
                break;
            }
        }
        double avg_price = total_price / avg_volume;
        auto key = std::to_string(i.type_id);
        json.Key(key.data(), (unsigned)key.size(), true);
        json.Double(avg_price);
    }
    json.EndObject();

    http::HttpResponse resp;
    resp.status_code = 200;
    resp.body.assign(
        (const uint8_t*)buffer.GetString(),
        (const uint8_t*)buffer.GetString() + buffer.GetSize());
    return resp;
}
