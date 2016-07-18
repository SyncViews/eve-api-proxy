#pragma once
#include "http/core/HttpRequest.hpp"
#include "http/core/HttpResponse.hpp"
namespace crest
{
    class Cache;
}


/**GET /bulk-market-orders
 * 
 * params:
 *   - buy = get buy orders
 *   - sell = get sell orders
 *   - region = array of region ids
 *   - type = array of type ids
 */
http::HttpResponse http_get_bulk_market_orders(crest::Cache &cache, http::HttpRequest &request);

