#pragma once
#include <string>
#include <vector>
class CrestCache;
struct MarketOrder
{
    MarketOrder() {}

    long long id;
    int duration;
    std::string issued;
    int station_id;
    std::string station_name;
    int min_volume;
    int volume;
    int volume_entered;
    std::string range;
    float price;
};
struct MarketOrderList
{
    MarketOrderList(bool buy, int region_id, int type_id)
        : buy(buy), region_id(region_id), type_id(type_id), orders()
    {}

    bool buy;
    int region_id;
    int type_id;
    std::vector<MarketOrder> orders;
};
/**Parse the CREST JSON data into an orders array*/
void parse_crest_orders(
    std::vector<MarketOrder> &out,
    const std::vector<uint8_t> &crest_data);
/**Populates the orders vectors in the MarketOrderLists.*/
void get_crest_bulk_market_orders(
    CrestCache &cache,
    std::vector<MarketOrderList> &order_lists);
/**Creates a JSON response from a list of MarketOrderList*/
std::string market_order_lists_json(const std::vector<MarketOrderList> &orders);
