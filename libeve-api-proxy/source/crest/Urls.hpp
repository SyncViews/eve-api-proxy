#pragma once
#include <string>
#include <sstream>
#include "../CppServers.hpp"

namespace crest
{
    inline std::string type_url(int type_id)
    {
        return "/inventory/types/" + std::to_string(type_id) + "/";
    }
    inline std::string type_full_url(int type_id)
    {
        std::stringstream ss;
        ss << "https://" << PCREST_HOST << type_url(type_id);
        return ss.str();
    }

    inline std::string market_orders_all_url(int region_id)
    {
        std::stringstream ss;
        ss << "/market/" << region_id << "/orders/all/";
        return ss.str();
    }
    inline std::string market_history_url(int region_id, int type_id)
    {
        std::stringstream ss;
        ss << "/market/" << region_id << "/history/?type=" << type_full_url(type_id);
        return ss.str();
    }
}
