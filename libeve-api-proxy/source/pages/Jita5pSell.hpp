#pragma once
#include "http/core/HttpRequest.hpp"
#include "http/core/HttpResponse.hpp"
namespace crest
{
    class Cache;
}

/**GET /jita-5p-sell
 *
 * params:
 *   - type = array of type ids
 */
http::HttpResponse http_get_jita_5p_sell(crest::Cache &cache, http::HttpRequest &request);
