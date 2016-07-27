#pragma once
#include <http/Request.hpp>
#include <http/Response.hpp>
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
http::Response http_get_bulk_market_orders(crest::Cache &cache, http::Request &request);

