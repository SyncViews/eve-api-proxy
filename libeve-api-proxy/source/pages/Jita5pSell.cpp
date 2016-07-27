#include "Precompiled.hpp"
#include "Errors.hpp"
#include "Jita5pSell.hpp"
#include "CrestBulkMarketOrders.hpp"
#include "model/EveRegions.hpp"
#include <iostream>
#include <chrono>
#include <json/Writer.hpp>
http::Response http_get_jita_5p_sell(crest::Cache &cache, http::Request &request)
{
    // Get params
    std::vector<MarketOrderList> order_sets;
    for (auto &i : request.url.query_param_list("type"))
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
    json::Writer json_writer;
    json_writer.start_obj();
    for (auto &i : order_sets)
    {
        //only jita 4-4
        std::vector<MarketOrder> orders;
        for (auto &j : i.orders)
        {
            if (j.station_id == EVE_JITA_4_4_ID) orders.push_back(j);
        }

        std::sort(orders.begin(), orders.end(),
            [](const MarketOrder &a, const MarketOrder &b) {
                return a.price < b.price;
            });
        int64_t total_volume = 0;
        for (auto &a : orders) total_volume += a.volume;

        if (!total_volume) continue; //no orders

        auto avg_volume = (total_volume + 20 - 1) / 20;
        auto remaining_volume = avg_volume;
        double total_price = 0;
        for (auto &a : orders)
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

        json_writer.prop(key, avg_price);
    }
    json_writer.end_obj();

    http::Response resp;
    resp.status_code(http::SC_OK);
    resp.body = json_writer.str();
    return resp;
}
