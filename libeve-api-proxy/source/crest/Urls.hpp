#pragma once
#include <string>
#include <sstream>

namespace crest
{
    extern const std::string CREST_HOST;
    static const uint16_t CREST_PORT = 443;

    inline std::string market_orders_all_url(int region_id)
    {
        std::stringstream ss;
        ss << "/market/" << region_id << "/orders/all/";
        return ss.str();
    }
}
