#pragma once
#include <http/Request.hpp>
#include <http/Response.hpp>
namespace crest
{
    class CacheOld;
}


/**GET /hub-buy-prices
 *
 * params:
 *   - type = array of type ids
 */
http::Response http_get_hub_buy_prices(crest::CacheOld &cache, http::Request &request);

/**GET /hub-sell-prices
 *
 * params:
 *   - type = array of type ids
 */
http::Response http_get_hub_sell_prices(crest::CacheOld &cache, http::Request &request);
