#include "Precompiled.hpp"
#include "MarketOrdersAll.hpp"
#include "MarketOrderSlim.hpp"
#include "ConnectionPool.hpp"
#include "Urls.hpp"
#include "Gzip.hpp"
#include "Log.hpp"

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

        void my_read_json(
            const GzipOut &json,
            unsigned *page_count,
            std::function<void(const MarketOrderSlim &order)> cb)
        {
            using namespace json;
            Parser parser(json.data.get(), json.data.get() + json.size);
            bool read_page_count = false;
            bool read_items = false;

            auto tok = parser.next();
            if (tok.type != Token::OBJ_START) throw ParseError("Expected object");
            do
            {
                tok = parser.next();
                if (tok.type != Token::STRING) throw ParseError("Expected object key");
                auto name = std::move(tok.str);

                tok = parser.next();
                if (tok.type != Token::KEY_SEP) throw ParseError("Expected ':'");

                if (name == "pageCount")
                {
                    if (read_page_count) throw ParseError("Multiple pageCount");
                    read_page_count = true;
                    read_json(parser, page_count);
                }
                else if (name == "items")
                {
                    if (read_items) throw ParseError("Multiple items");
                    read_items = true;
                    
                    tok = parser.next();
                    if (tok.type != Token::ARR_START) throw ParseError("Expected array");
                    if (!parser.try_next_arr_end())
                    {
                        do
                        {
                            MarketOrderSlim order;
                            read_json(parser, &order);
                            cb(order);

                            tok = parser.next();
                        }
                        while (tok.type == Token::ELEMENT_SEP);
                        if (tok.type != Token::ARR_END) throw ParseError("Expected array end");
                    }
                }
                else skip_value(parser);

                tok = parser.next();
            }
            while (tok.type == Token::ELEMENT_SEP);
            if (tok.type != Token::OBJ_END) throw ParseError("Expected object end");
            tok = parser.next();
            if (tok.type != Token::END) throw ParseError("Expected JSON end");
            if (!read_items) throw ParseError("Missing items");
            if (!read_page_count) throw ParseError("Missing pageCount");
        }
    
    }

    std::vector<MarketOrderSlim> get_market_orders_all(
        ConnectionPool &conn_pool,
        int region_id)
    {
        std::vector<MarketOrderSlim> out;
        get_market_orders_all(conn_pool, region_id,
            [&out](const MarketOrderSlim &order) { out.push_back(order); });
        return out;
    }

    void get_market_orders_all(
        ConnectionPool &conn_pool,
        int region_id,
        std::function<void(const MarketOrderSlim &order)> cb)
    {
        auto url = market_orders_all_url(region_id);
        unsigned page_count;
        //Get first page
        {
            http::AsyncRequest first_page_req;
            send_page_request(first_page_req, conn_pool, url, 1);
            auto first_page_resp = first_page_req.wait();
            auto decompressed = gzip_decompress((const uint8_t*)first_page_resp->body.data(), first_page_resp->body.size());
            first_page_resp->body.clear();
            my_read_json(decompressed, &page_count, cb);
        }
        if (page_count > 1)
        {
            //Send all requests
            std::unique_ptr<http::AsyncRequest[]> page_reqs(new http::AsyncRequest[page_count - 1]);
            for (unsigned i = 0; i < page_count - 1; ++i)
            {
                send_page_request(page_reqs[i], conn_pool, url, i + 2);
            }
            //Process responses
            for (unsigned i = 0; i < page_count - 1; ++i)
            {
                unsigned ignore;
                auto resp = page_reqs[i].wait();
                auto decompressed = gzip_decompress((const uint8_t*)resp->body.data(), resp->body.size());
                resp->body.clear();
                my_read_json(decompressed, &ignore, cb);
            }
        }
        log_info() << "Completed get_market_orders_all for " << region_id << std::endl;
    }
}
