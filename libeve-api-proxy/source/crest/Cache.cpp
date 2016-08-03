#include "Precompiled.hpp"
#include "Cache.hpp"
#include "MarketOrdersAll.hpp"

namespace crest
{
    namespace
    {
        using namespace std::placeholders;
    }
    Cache::Cache()
        : conn_pool()
        , market_orders(std::bind(&get_market_orders_all_cb, std::ref(conn_pool), _1, _2))
    {
    }

    Cache::~Cache()
    {
        exit();
    }

    void Cache::exit()
    {
        market_orders.exit();
        conn_pool.exit();
    }
}
