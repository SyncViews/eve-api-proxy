#pragma once
#include "http/core/HttpRequest.hpp"
#include "http/core/HttpResponse.hpp"
namespace crest
{
    class Cache;
}


/**GET /hub-buy-prices
 *
 * params:
 *   - type = array of type ids
 */
http::HttpResponse http_get_hub_buy_prices(crest::Cache &cache, http::HttpRequest &request);

/**GET /hub-sell-prices
 *
 * params:
 *   - type = array of type ids
 */
http::HttpResponse http_get_hub_sell_prices(crest::Cache &cache, http::HttpRequest &request);
