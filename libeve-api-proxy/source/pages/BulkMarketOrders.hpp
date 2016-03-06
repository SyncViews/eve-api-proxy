#pragma once
#include "http/core/HttpRequest.hpp"
#include "http/core/HttpResponse.hpp"
class CrestCache;

/**GET /bulk-market-orders
 * 
 * params:
 *   - buy = get buy orders
 *   - sell = get sell orders
 *   - region = array of region ids
 *   - type = array of type ids
 */
http::HttpResponse http_get_bulk_market_orders(CrestCache &cache, http::HttpRequest &request);

