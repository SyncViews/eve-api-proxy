#include "Precompiled.hpp"
#include "Errors.hpp"
#include "Jita5pSell.hpp"
#include "crest/Cache.hpp"
#include "model/EveRegions.hpp"
#include "lib/Params.hpp"
#include <iostream>
#include <chrono>
#include <json/Writer.hpp>
http::Response http_get_jita_5p_sell(crest::Cache &cache, http::Request &request)
{
    // Get params
    std::vector<int> types = params_inv_types(request);

    // Build requests
    log_info() << "GET /jita-5p-sell with " << types.size() << " types" << std::endl;
    
    // Make requests
    auto start = std::chrono::high_resolution_clock::now();
    json::Writer json_writer;
    json_writer.start_obj();
    for (auto type : types)
    {
        auto orders = cache.get_orders(EVE_THE_FORGE_ID, type, false);
        std::vector<const crest::MarketOrderSlim*> jita_orders;
        for (auto &j : *orders)
        {
            if (j.station_id == EVE_JITA_4_4_ID) jita_orders.push_back(&j);
        }

        //total volume
        int64_t total_volume = 0;
        for (auto &a : jita_orders) total_volume += a->volume;
        if (!total_volume) continue; //no orders

        //sort by price
        std::sort(jita_orders.begin(), jita_orders.end(),
            [](const crest::MarketOrderSlim* a, const crest::MarketOrderSlim* b) {
            return a->price < b->price;
        });

        //5% price
        auto avg_volume = (total_volume + 20 - 1) / 20;
        auto remaining_volume = avg_volume;
        double total_price = 0;
        for (auto &a : jita_orders)
        {
            if (remaining_volume > a->volume)
            {
                remaining_volume -= a->volume;
                total_price += a->price * a->volume;
            }
            else
            {
                total_price += a->price * remaining_volume;
                break;
            }
        }
        //output
        double avg_price = total_price / avg_volume;
        json_writer.prop(std::to_string(type), avg_price);

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
