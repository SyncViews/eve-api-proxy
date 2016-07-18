#pragma once
#include "http/core/HttpRequest.hpp"
#include "http/core/HttpResponse.hpp"
namespace crest
{
    class Cache;
}

/**GET /bulk-market-history
 *
 * params:
 *   - region = array of region ids
 *   - type = array of type ids
 */
http::HttpResponse http_get_bulk_market_history(crest::Cache &cache, http::HttpRequest &request);
/**GET /bulk-market-history-aggregated
 * 
 * Returns just a single value for each type, with multiple regions combined as min/max/sum/averages
 * params:
 *   - region = array of region ids
 *   - type = array of type ids
 */
http::HttpResponse http_get_bulk_market_history_aggregated(crest::Cache &cache, http::HttpRequest &request);

