#include "Precompiled.hpp"
#include "Errors.hpp"
#include "BulkMarketOrders.hpp"
//#include "CrestBulkMarketOrders.hpp"
#include "crest/Cache.hpp"
#include "crest/MarketOrderSlim.hpp"
#include "model/EveRegions.hpp"
#include "lib/Params.hpp"
#include "../String.hpp"
#include <iostream>
#include <chrono>
#include <unordered_set>
namespace
{
    struct OrderSet
    {
        bool buy;
        int region_id;
        int type_id;
        std::future<std::shared_ptr<const crest::MarketOrdersSlim>> future;

        OrderSet(bool buy, int region_id, int type_id)
            : buy(buy), region_id(region_id), type_id(type_id)
            , future()
        {}
    };
}
http::Response http_get_bulk_market_orders(crest::Cache &cache, http::Request &request)
{
    // Get params
    std::vector<OrderSet> order_sets;
    std::vector<int> regions;

    auto order_params = request.url.query_param_list("order");
    if (order_params.empty())
    {
        bool buy = request.url.has_query_param("buy");
        bool sell = request.url.has_query_param("sell");

        if (!buy && !sell && order_params.empty()) return http_bad_request(request, "At least 'buy' or 'sell' param required.");

        regions = params_regions(request);
        std::vector<int> types = params_inv_types(request);

        // Build requests
        size_t count = ((buy ? 1 : 0) + (sell ? 1 : 0)) * regions.size() * types.size();

        order_sets.reserve(count);
        for (auto i : regions)
        {
            for (auto j : types)
            {
                if (buy) order_sets.emplace_back(true, i, j);
                if (sell) order_sets.emplace_back(false, i, j);
            }
        }
    }
    else
    {
        std::unordered_set<int> regions_set;
        for (auto &param : order_params)
        {
            auto parts = splitString(param, ',');
            if (parts.size() != 3) return http_bad_request(request, "Invalid order param");
            bool buy = parts[0] == "buy";
            int region_id = std::stoi(parts[1]);
            int type_id = std::stoi(parts[2]);
            regions_set.insert(region_id);
            order_sets.emplace_back(buy, region_id, type_id);
        }
        for (int region : regions_set) regions.push_back(region);
    }
    log_info() << "GET /bulk-market-orders with " << order_sets.size() << " order sets" << std::endl;
    // Make requests
    auto start = std::chrono::high_resolution_clock::now();
    cache.update_region_orders(regions);
    
    // Process results
    json::Writer writer;
    writer.start_arr();
    for (auto &order_set : order_sets)
    {
        //order_set.future.wait();
        //auto orders = order_set.future.get();
        auto orders = cache.get_orders(order_set.region_id, order_set.type_id, order_set.buy);

        writer.start_obj();
        writer.prop("buy", order_set.buy);
        writer.prop("region", order_set.region_id);
        writer.prop("type", order_set.type_id);
        writer.prop("items", *orders);
        writer.end_obj();
    }
    writer.end_arr();

    auto end = std::chrono::high_resolution_clock::now();
    log_info() << "Cache lookup took "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
        << "ms." << std::endl;
    // Response
    http::Response resp;
    resp.status_code(http::SC_OK);
    resp.body = std::move(writer.str());
    return resp;
}
