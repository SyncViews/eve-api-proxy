#include "Precompiled.hpp"
#include "MarketOrdersAll.hpp"
#include "MarketOrderSlim.hpp"
#include "ConnectionPool.hpp"
#include "Urls.hpp"
#include "Gzip.hpp"

namespace crest
{
    namespace
    {
        struct Page
        {
            std::vector<MarketOrderSlim> items;
            int page_count;
            int total_count;
        };
        inline void read_json(json::Parser &parser, Page *out)
        {
            static const auto reader = json::ObjectFieldReader<Page, json::IgnoreUnknown>()
                .add<decltype(Page::items), &Page::items>("items")
                .add<decltype(Page::page_count), &Page::page_count>("pageCount")
                .add<decltype(Page::total_count), &Page::total_count>("totalCount")
                ;
            reader.read(parser, out);
        }
        void send_page_request(http::AsyncRequest &req, ConnectionPool &conn_pool, const std::string &base_url, int page)
        {
            req.method = http::GET;
            req.raw_url = base_url;
            req.headers.add("Accept", "application/vnd.ccp.eve.MarketOrderCollectionSlim-v1+json");
            if (page > 1) req.raw_url += "?page=" + std::to_string(page);

            conn_pool.queue(&req);
        }
    }
    std::vector<MarketOrderSlim> get_market_orders_all(
        ConnectionPool &conn_pool,
        int region_id)
    {
        auto url = market_orders_all_url(region_id);
        //Get first page
        http::AsyncRequest first_page_req;
        send_page_request(first_page_req, conn_pool, url, 1);
        auto first_page_resp = first_page_req.wait();
        auto page = json::read_json<Page>(gzip_decompress(first_page_resp->body));
        int page_count = page.page_count;

        //Send all requests
        std::unique_ptr<http::AsyncRequest[]> page_reqs(new http::AsyncRequest[page_count - 1]);
        for (int i = 0; i < page_count - 1; ++i)
        {
            send_page_request(page_reqs[i], conn_pool, url, i + 2);
        }
        //Process responses
        for (int i = 0; i < page_count - 1; ++i)
        {
            auto resp = page_reqs[i].wait();
            json::read_json(gzip_decompress(resp->body), &page);
        }

        return std::move(page.items);
    }
}
