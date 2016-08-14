#pragma once

namespace crest
{
    class MarketOrdersCache;
    
    void update_market_summary_csv(MarketOrdersCache &cache);
    std::string get_market_summary_csv();
}
