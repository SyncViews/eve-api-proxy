#include "Precompiled.hpp"
#include "Cache.hpp"
#include "MarketOrdersAll.hpp"

namespace crest
{
    Cache::Cache()
        : conn_pool()
        , market_orders(std::bind(&Cache::get_region_orders, this, std::placeholders::_1))
    {
    }

    Cache::~Cache()
    {
    }

    MarketOrdersSlim Cache::get_region_orders(int region_id)
    {
        return get_market_orders_all(conn_pool, region_id);
    }
}
