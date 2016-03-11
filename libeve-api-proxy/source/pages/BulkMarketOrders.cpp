#include "Precompiled.hpp"
#include "Errors.hpp"
#include "BulkMarketOrders.hpp"
#include "CrestBulkMarketOrders.hpp"
#include "EveRegions.hpp"
#include "lib/Params.hpp"
#include <iostream>
#include <chrono>
http::HttpResponse http_get_bulk_market_orders(CrestCache &cache, http::HttpRequest &request)
{
    // Get params
    bool buy = request.url.has_query_param("buy");
    bool sell = request.url.has_query_param("sell");

    if (!buy && !sell) return http_bad_request(request, "At least 'buy' or 'sell' param required.");

    std::vector<int> regions = params_regions(request);
    std::vector<int> types = params_inv_types(request);

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
