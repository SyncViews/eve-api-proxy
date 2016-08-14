#pragma once
#include <http/Request.hpp>
#include <http/Response.hpp>
namespace crest
{
    class ConnectionPool;
}

/**GET /bulk-market-history
 *
 * params:
 *   - region = array of region ids
 *   - type = array of type ids
 */
http::Response http_get_bulk_market_history(crest::ConnectionPool &pool, http::Request &request);
/**GET /bulk-market-history-aggregated
 * 
 * Returns just a single value for each type, with multiple regions combined as min/max/sum/averages
 * params:
 *   - region = array of region ids
 *   - type = array of type ids
 */
http::Response http_get_bulk_market_history_aggregated(crest::ConnectionPool &pool, http::Request &request);

