#include "Precompiled.hpp"
#include "MarketSummaryCsv.hpp"
#include "MarketOrdersCache.hpp"
#include "FivePercentPrice.hpp"
#include "model/EveRegions.hpp"
#include "Log.hpp"
#include <fstream>
#include <iomanip>
#include <mutex>
#include <boost/filesystem.hpp>

namespace crest
{
    namespace
    {
        void csv_header(std::ostream &os)
        {
            os << "region,type,buy,order count,total volume,5% price,mean price" << std::endl;
        }
        void csv_row(std::ostream &os, int region_id, int type_id, bool buy, MarketOrdersSlim &orders)
        {
            unsigned long long total_volume = 0;
            double mean_price = 0;
            for (auto &order : orders)
            {
                total_volume += order.volume;
                mean_price += order.price * order.volume;
            }
            mean_price /= total_volume;

            auto fivep_price = five_percent_price(orders, buy, total_volume);

            os << region_id << ',' << type_id << ',' << (buy ? "buy" : "sell") << ',';
            os << orders.size() << ',' << total_volume << ',' << fivep_price << ',' << mean_price << std::endl;
        }
        void csv_region(std::ostream &os, MarketOrdersCache &cache, int region_id)
        {
            auto region = cache.get_orders(region_id);
            for (auto &type : region)
            {
                csv_row(os, region_id, type.type_id, type.buy, type.orders);
            }
        }
        void csv(std::ostream &os, MarketOrdersCache &cache)
        {
            csv_header(os);
            for (auto region_id : EVE_REGION_IDS)
            {
                csv_region(os, cache, region_id);
            }
        }

        static const std::string CACHE_DIR = "cache/";
        static const std::string CACHE_FILE = CACHE_DIR + "market_summary.csv";
        static const time_t CACHE_TIME = 60 * 60;
        std::mutex mutex;
        time_t last_updated = 0;

        void update(MarketOrdersCache &cache)
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (time(nullptr) < last_updated + CACHE_TIME) return;

            try
            {
                log_info() << "Updating " << CACHE_FILE << std::endl;
                boost::filesystem::create_directories(CACHE_DIR);
                std::fstream os(CACHE_FILE, std::ios::out | std::ios::trunc | std::ios::binary);
                if (!os) throw std::runtime_error("Failed to open");
                os << std::fixed << std::setprecision(2);
                csv(os, cache);
                if (!os) throw std::runtime_error("Error writing");
                last_updated = time(nullptr);
                log_info() << "Updated " << CACHE_FILE << std::endl;
            }
            catch (const std::exception &e)
            {
                log_error() << "Error updating " << CACHE_FILE << ": " << e.what() << std::endl;
            }
        }
        std::string fetch()
        {
            std::unique_lock<std::mutex> lock(mutex);
            std::ifstream is(CACHE_FILE, std::ios::in | std::ios::binary);
            if (!is) throw std::runtime_error("Failed to open " + CACHE_FILE);

            is.seekg(0, std::ios::end);
            auto size = (size_t)is.tellg();
            is.seekg(0, std::ios::beg);

            std::string out;
            out.resize(size);
            is.read(&out[0], size);

            if (is.gcount() != size) throw std::runtime_error("Error reading " + CACHE_FILE);

            return out;
        }
    }

    void update_market_summary_csv(MarketOrdersCache &cache)
    {
        update(cache);
    }
    std::string get_market_summary_csv()
    {
        return fetch();
    }
}
