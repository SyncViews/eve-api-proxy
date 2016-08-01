#pragma once
#include <functional>
#include <vector>
namespace crest
{
    class ConnectionPool;
    struct MarketOrderSlim;
    typedef std::function<void(const MarketOrderSlim &order)> GetMarketOrdersAllCb;
    std::vector<MarketOrderSlim> get_market_orders_all(
        ConnectionPool &conn_pool,
        int region_id);
    void get_market_orders_all(
        ConnectionPool &conn_pool,
        int region_id,
        GetMarketOrdersAllCb cb);
    inline void get_market_orders_all_cb(
        ConnectionPool &conn_pool,
        int region_id,
        GetMarketOrdersAllCb cb)
    {
        get_market_orders_all(conn_pool, region_id, cb);
    }
}
