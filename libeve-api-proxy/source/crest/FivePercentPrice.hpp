#pragma once
#include "MarketOrderSlim.hpp"
#include <algorithm>
namespace crest
{
    namespace detail
    {
        inline const MarketOrderSlim& deref_order(const MarketOrderSlim &order)
        {
            return order;
        }
        inline const MarketOrderSlim& deref_order(const MarketOrderSlim *order)
        {
            return *order;
        }
    }
    /**Calculate the 5% market order price. Container will be sorted by price.*/
    template<typename T>
    double five_percent_price(T &orders, bool buy, unsigned long long total_volume)
    {
        typedef typename T::value_type value_type;
        if (!total_volume) return NAN;
        if (buy)
        {
            std::sort(orders.begin(), orders.end(), [](const value_type &a, const value_type &b) {
                return detail::deref_order(a).price > detail::deref_order(b).price;
            });
        }
        else
        {
            std::sort(orders.begin(), orders.end(), [](const value_type &a, const value_type &b) {
                return detail::deref_order(a).price < detail::deref_order(b).price;
            });
        }

        auto avg_volume = (total_volume + 20 - 1) / 20;
        auto remaining_volume = avg_volume;
        double total_price = 0;
        for (auto &a : orders)
        {
            auto &order = detail::deref_order(a);
            if (remaining_volume > order.volume)
            {
                remaining_volume -= order.volume;
                total_price += order.price * order.volume;
            }
            else
            {
                total_price += order.price * remaining_volume;
                break;
            }
        }
        double avg_price = total_price / avg_volume;
        return avg_price;
    }
    template<typename T>
    double five_percent_price(T &container, bool buy)
    {
        typedef typename T::value_type value_type;
        unsigned long long total_volume = 0;
        for (auto &order : container)
        {
            total_volume += detail::deref_order(order).volume;
        }
        return five_percent_price(container, buy, total_volume);
    }
}
