#pragma once
#include "http/core/HttpRequest.hpp"
#include "http/core/HttpResponse.hpp"
class CrestCache;

/**GET /bulk-market-history
 *
 * params:
 *   - region = array of region ids
 *   - type = array of type ids
 */
http::HttpResponse http_get_bulk_market_history(CrestCache &cache, http::HttpRequest &request);
/**GET /bulk-market-history-aggregated
 * 
 * Returns just a single value for each type, with multiple regions combined as min/max/sum/averages
 * params:
 *   - region = array of region ids
 *   - type = array of type ids
 */
http::HttpResponse http_get_bulk_market_history_aggregated(CrestCache &cache, http::HttpRequest &request);

