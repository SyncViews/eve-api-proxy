#pragma once
#include <http/Request.hpp>
#include <http/Response.hpp>
namespace crest
{
    class Cache;
}

/**GET /jita-5p-sell
 *
 * params:
 *   - type = array of type ids
 */
http::Response http_get_jita_5p_sell(crest::Cache &cache, http::Request &request);
