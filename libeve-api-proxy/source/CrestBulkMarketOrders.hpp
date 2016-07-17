#pragma once
#include <string>
#include <vector>
#include "model/MarketOrder.hpp"
class CrestCache;

/**Populates the orders vectors in the MarketOrderLists.*/
void get_crest_bulk_market_orders(
    CrestCache &cache,
    std::vector<MarketOrderList> &order_lists);
