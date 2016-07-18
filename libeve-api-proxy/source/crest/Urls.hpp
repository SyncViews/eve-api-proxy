#pragma once
#include <string>
#include <sstream>

namespace crest
{
    inline std::string market_orders_all_url(int region_id)
    {
        std::stringstream ss;
        ss << "/market/" << region_id << "/orders/all/";
        return ss.str();
    }
}
