#include "Precompiled.hpp"
#include "Errors.hpp"
#include "Jita5pSell.hpp"
#include "crest/Cache.hpp"
#include "crest/FivePercentPrice.hpp"
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
    // Response
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

        auto avg_price = crest::five_percent_price(jita_orders, false);

        if (std::isnan(avg_price))
        {
            json_writer.key(std::to_string(type));
            json_writer.null();
        }
        else json_writer.prop(std::to_string(type), avg_price);

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
