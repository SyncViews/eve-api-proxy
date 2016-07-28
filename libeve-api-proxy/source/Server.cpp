#include "Precompiled.hpp"
#include "Server.hpp"
#include "Error.hpp"

#include "pages/Errors.hpp"
#include "pages/BulkMarketHistory.hpp"
#include "pages/BulkMarketOrders.hpp"
#include "pages/HubPrices.hpp"
#include "pages/Jita5pSell.hpp"

#include <iostream>

Server::Server()
    : http::CoreServer(5000)
    , crest_cache()
{
}

Server::~Server()
{}

void Server::run()
{
    set_thread_name("server-main");
    http::CoreServer::run();
}

http::Response Server::handle_request(http::Request &request)
{
    http::Response response;
    log_info() << request.method << " " << request.raw_url << std::endl;
    try
    {
        if (request.url.path == "/bulk-market-history")
        {
            response = http_get_bulk_market_history(crest_cache, request);
        }
        else if (request.url.path == "/bulk-market-history-aggregated")
        {
            response = http_get_bulk_market_history_aggregated(crest_cache, request);
        }
        else if (request.url.path == "/bulk-market-orders")
        {
            response = http_get_bulk_market_orders(cache, request);
        }
        else if (request.url.path == "/hub-buy-prices")
        {
            response = http_get_hub_buy_prices(crest_cache, request);
        }
        else if (request.url.path == "/hub-sell-prices")
        {
            response = http_get_hub_sell_prices(crest_cache, request);
        }
        else if (request.url.path == "/jita-5p-sell")
        {
            response = http_get_jita_5p_sell(cache, request);
        }
        else response = http_simple_error_page(request, 404, request.url.path + " was not found");
    }
    catch (const http::BadRequest &err)
    {
        response = http_bad_request(request, err.what());
    }
    catch (const http::ErrorResponse &err)
    {
        response = http_simple_error_page(request, err.status_code(), err.what());
    }


    response.headers.set_default("Content-Type", "application/json; charset=UTF-8");
    return response;
}

http::Response Server::parser_error_page(const http::ParserError &err)
{
    int sc = err.status_code();
    if (!sc) sc = http::SC_BAD_REQUEST;


    http::Response response;
    response.status_code(sc);
    response.headers.set("Content-Type", "text/plain");
    response.body = err.what();
    return response;
}
