#include "Precompiled.hpp"
#include "Errors.hpp"
#include "BulkMarketOrders.hpp"
#include "CrestBulkMarketOrders.hpp"
#include "EveRegions.hpp"
#include <iostream>
#include <chrono>
http::HttpResponse http_get_bulk_market_orders(CrestCache &cache, http::HttpRequest &request)
{
    // Get params
    bool buy = request.url.has_query_param("buy");
    bool sell = request.url.has_query_param("sell");

    if (!buy && !sell) return http_bad_request(request, "At least 'buy' or 'sell' param required.");

    std::vector<int> regions;
    if (request.url.query_param("region") == "all")
    {
        regions.assign(EVE_REGION_IDS, EVE_REGION_IDS + sizeof(EVE_REGION_IDS) / sizeof(EVE_REGION_IDS[0]));
    }
    else
    {
        for (auto &i : request.url.query_array_param("region"))
        {
            regions.push_back(std::stoi(i));
        }
        if (regions.empty()) return http_bad_request(request, "At least one region required.");
    }

    std::vector<int> types;
    for (auto &i : request.url.query_array_param("type"))
    {
        types.push_back(std::stoi(i));
    }
    if (types.empty()) return http_bad_request(request, "At least one type required.");

    // Build requests
    size_t count = ((buy ? 1 : 0) + (sell ? 1 : 0)) * regions.size() * types.size();
    log_info() << "GET /bulk-market-orders with " << count << " order sets" << std::endl;

    std::vector<MarketOrderList> order_sets;
    order_sets.reserve(count);
    for (auto i : regions)
    {
        for (auto j : types)
        {
            if (buy) order_sets.emplace_back(true, i, j);
            if (sell) order_sets.emplace_back(false, i, j);
        }
    }
    // Make requests
    auto start = std::chrono::high_resolution_clock::now();
    get_crest_bulk_market_orders(cache, order_sets);
    auto end = std::chrono::high_resolution_clock::now();
    log_info() << "Cache lookup took "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
        << "ms." << std::endl;
    // Response
    std::string resp_str = market_order_lists_json(order_sets);


    http::HttpResponse resp;
    resp.status_code = 200;
    resp.body.assign(
        (const uint8_t*)resp_str.data(),
        (const uint8_t*)resp_str.data() + resp_str.size());
    return resp;
}
