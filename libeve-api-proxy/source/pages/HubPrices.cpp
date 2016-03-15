#include "Precompiled.hpp"
#include "HubPrices.hpp"
#include "CrestBulkMarketOrders.hpp"
#include "EveRegions.hpp"
#include "lib/Params.hpp"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

template<bool buy>
http::HttpResponse http_get_hub_prices(CrestCache &cache, http::HttpRequest &request)
{
    static const int REGIONS[] = { EVE_THE_FORGE_ID, EVE_HEIMATAR_ID, EVE_METROPOLIS_ID, EVE_DOMAIN_ID, EVE_SINQ_LAISON_ID };
    static const int REGION_COUNT = sizeof(REGIONS) / sizeof(REGIONS[0]);
    static const int STATIONS[] = { EVE_JITA_4_4_ID, EVE_RENS_HUB_ID, EVE_HEK_HUB_ID, EVE_AMARR_HUB_ID, EVE_DODIXIE_HUB_ID };

    std::vector<int> types = params_inv_types(request);
    std::vector<MarketOrderList> order_sets;
    for (auto type : types)
    {
        for (auto region : REGIONS)
        {
            order_sets.emplace_back(buy, region, type);
        }
    }

    log_info() << "GET /hub-(buy|sell)-prices with " << order_sets.size() << " order_sets" << std::endl;
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
    auto it = order_sets.begin();
    for (auto type : types)
    {
        auto stype = std::to_string(type);
        json.Key(stype.data(), (unsigned)stype.size(), true);
        json.StartObject();
        for (int j = 0; j < REGION_COUNT; ++j)
        {
            auto region = REGIONS[j];
            auto station = STATIONS[j];
            assert(it->type_id == type && it->region_id == region);
            assert(it->buy == buy);
            bool found = false;
            float best = 0;
            for (auto &order : it->orders)
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
                auto sregion = std::to_string(region);
                json.Key(sregion.data(), (unsigned)sregion.size(), true);
                json.Double(best);
            }
            ++it;
        }
        json.EndObject();
    }
    json.EndObject();

    http::HttpResponse resp;
    resp.status_code = 200;
    resp.body.assign(
        (const uint8_t*)buffer.GetString(),
        (const uint8_t*)buffer.GetString() + buffer.GetSize());
    return resp;
}

http::HttpResponse http_get_hub_buy_prices(CrestCache &cache, http::HttpRequest &request)
{
    return http_get_hub_prices<true>(cache, request);
}
http::HttpResponse http_get_hub_sell_prices(CrestCache &cache, http::HttpRequest &request)
{
    return http_get_hub_prices<false>(cache, request);
}
