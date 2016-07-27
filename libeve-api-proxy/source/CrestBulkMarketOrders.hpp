#pragma once
#include <string>
#include <vector>
#include "model/MarketOrder.hpp"
namespace crest
{
    class CacheOld;
}

/**Populates the orders vectors in the MarketOrderLists.*/
void get_crest_bulk_market_orders(
    crest::CacheOld &cache,
    std::vector<MarketOrderList> &order_lists);
