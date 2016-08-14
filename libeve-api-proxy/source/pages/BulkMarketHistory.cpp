#include "Precompiled.hpp"
#include "Errors.hpp"
#include "BulkMarketHistory.hpp"
#include "crest/ConnectionPool.hpp"
#include "crest/JsonHelpers.hpp"
#include "crest/Urls.hpp"
#include "lib/Params.hpp"
#include "Gzip.hpp"
#include <iostream>
#include <chrono>
#include <map>
#include <json/Reader.hpp>
#include <json/Writer.hpp>
#include <json/Copy.hpp>
#include "model/MarketHistoryDay.hpp"

std::vector<http::AsyncRequest> get_bulk_market_history(
    crest::ConnectionPool &pool, http::Request &request,
    const std::vector<int> &regions, const std::vector<int> &types)
{
    // Build requests
    size_t count = regions.size() * types.size();
    log_info() << "GET " << request.url.path << " with " << count << " histories" << std::endl;

    std::vector<http::AsyncRequest> requests;
    requests.resize(count);
    size_t k = 0;
    for (auto i : types)
    {
        for (auto j : regions)
        {
            auto &req = requests[k++];
            req.method = http::GET;
            req.raw_url = crest::market_history_url(j, i);
            pool.queue(&req);
        }
    }

    return requests;
}

http::Response http_get_bulk_market_history(crest::ConnectionPool &pool, http::Request &request)
{
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<int> regions = params_regions(request);
    std::vector<int> types = params_inv_types(request);
    auto requests = get_bulk_market_history(pool, request, regions, types);

    //Results
    json::Writer json_writer;
    json_writer.start_arr();
    size_t k = 0;
    for (auto i : types)
    {
        for (auto j : regions)
        {
            auto &req = requests[k++];
            auto resp = req.wait();
            auto data = gzip_decompress(resp->body);

            json_writer.start_obj();
            json_writer.prop("region", j);
            json_writer.prop("type", i);
            json_writer.end_obj();

            json::Parser parser(
                (const char*)data.data(),
                (const char*)data.data() + data.size());
            crest::read_crest_items(parser, [&parser, &json_writer]() -> void
            {
                json::copy(json_writer, parser);
            });
        }
    }
    json_writer.end_arr();
    auto end = std::chrono::high_resolution_clock::now();
    log_info() << "Cache lookup took "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
        << "ms." << std::endl;

    http::Response resp;
    resp.status_code(http::SC_OK);
    resp.body = json_writer.str();
    return resp;
}

http::Response http_get_bulk_market_history_aggregated(crest::ConnectionPool &pool, http::Request &request)
{
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<int> regions = params_regions(request);
    std::vector<int> types = params_inv_types(request);
    auto requests = get_bulk_market_history(pool, request, regions, types);

    //Results
    json::Writer json_writer;
    json_writer.start_obj();
    size_t k = 0;
    for (auto i : types)
    {
        std::map<std::string, MarketHistoryDay> days;
        for (auto j : regions)
        {
            auto &req = requests[k++];
            auto resp = req.wait();

            auto data = gzip_decompress(resp->body);
            auto region_days = crest::read_crest_items<std::vector<MarketHistoryDay>>(data);

            for (auto &day : region_days)
            {
                auto ret = days.emplace(day.date, day);
                if (ret.second)
                {
                    auto &added = ret.first->second;
                    added.avg_price *= added.volume;
                }
                else
                {
                    auto &aggregate = ret.first->second;
                    if (day.high_price > aggregate.high_price) aggregate.high_price = day.high_price;
                    if (day.low_price < aggregate.low_price) aggregate.low_price = day.low_price;
                    aggregate.order_count += day.order_count;
                    aggregate.volume += day.volume;
                    aggregate.avg_price += day.avg_price * day.volume;
                }
            }
        }

        if (!days.empty())
        {
            json_writer.key(std::to_string(i));
            json_writer.start_arr();

            for (auto &day : days)
            {
                day.second.avg_price /= day.second.volume; //avg_price was a sum in previous loop
                json_writer.value(day.second);
            }

            json_writer.end_arr();
        }
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
