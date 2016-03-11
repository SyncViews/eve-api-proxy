#pragma once
#include "http/core/HttpRequest.hpp"
#include "http/core/HttpResponse.hpp"
class CrestCache;

/**GET /jita-5p-sell
 *
 * params:
 *   - type = array of type ids
 */
http::HttpResponse http_get_jita_5p_sell(CrestCache &cache, http::HttpRequest &request);
