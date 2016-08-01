#include "Precompiled.hpp"
#include "HubPrices.hpp"
#include "crest/Cache.hpp"
#include "model/EveRegions.hpp"
#include "lib/Params.hpp"
#include <json/Writer.hpp>

namespace
{
    static const int REGIONS[] = { EVE_THE_FORGE_ID, EVE_HEIMATAR_ID, EVE_METROPOLIS_ID, EVE_DOMAIN_ID, EVE_SINQ_LAISON_ID };
    static const int REGION_COUNT = sizeof(REGIONS) / sizeof(REGIONS[0]);
    static const int STATIONS[] = { EVE_JITA_4_4_ID, EVE_RENS_HUB_ID, EVE_HEK_HUB_ID, EVE_AMARR_HUB_ID, EVE_DODIXIE_HUB_ID };

    struct OrderSet
    {
        int region_id;
        int type_id;
        std::future<std::shared_ptr<const crest::MarketOrdersSlim>> future;

        OrderSet(int region_id, int type_id)
            : region_id(region_id), type_id(type_id)
            , future()
        {}
    };
}

template<bool buy>
http::Response http_get_hub_prices(crest::Cache &cache, http::Request &request)
{

    std::vector<int> types = params_inv_types(request);
    std::vector<std::future<std::shared_ptr<const crest::MarketOrdersSlim>>> futures;
    futures.reserve(types.size() * REGION_COUNT);

    log_info() << "GET /hub-(buy|sell)-prices with " << (types.size() * REGION_COUNT) << " order_sets" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    //Make requests
    cache.update_region_orders(REGIONS, REGION_COUNT);
    // Process results
    json::Writer writer;
    writer.start_arr();
    for (size_t i = 0; i < types.size(); ++i)
    {
        auto type = types[i];
        writer.key(std::to_string(type));
        writer.start_obj();
        for (int j = 0; j < REGION_COUNT; ++j)
        {
            auto region = REGIONS[j];
            auto station = STATIONS[j];

            auto orders = cache.get_orders(region, type, buy);

            bool found = false;
            float best = 0;
            for (auto &order : *orders)
            {
                if (order.station_id == station)
                {
                    if (!found || (buy && order.price > best) || (!buy && order.price < best))
                    {
                        found = true;
                        best = order.price;
                    }
                }
            }
            if (found)
            {
                //output
                writer.prop(std::to_string(region), best);
            }
        }
        writer.end_obj();
    }
    writer.end_arr();

    auto end = std::chrono::high_resolution_clock::now();
    log_info() << "Cache lookup took "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
        << "ms." << std::endl;

    http::Response resp;
    resp.status_code(http::SC_OK);
    resp.body = writer.str();
    return resp;
}

http::Response http_get_hub_buy_prices(crest::Cache &cache, http::Request &request)
{
    return http_get_hub_prices<true>(cache, request);
}
http::Response http_get_hub_sell_prices(crest::Cache &cache, http::Request &request)
{
    return http_get_hub_prices<false>(cache, request);
}
