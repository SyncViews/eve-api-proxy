#pragma once
#include "CrestHttpRequest.hpp"
#include <condition_variable>
#include <ctime>
#include <mutex>
#include <string>
#include <vector>
class CrestRequestFailed : public std::runtime_error
{
public:
    CrestRequestFailed() : std::runtime_error("A request to CREST failed") {}
};
struct CrestCacheEntry
{
    enum Status
    {
        NEW,
        /**Has valid cache data*/
        HIT,
        /**Got updated data from CCP.*/
        UPDATED,
        /**Update is in-progress. Any one that gets a cache entry in this state should likely
         * wait on the update_wait condition variable.
         */
        UPDATING,
        /**Has valid data, but cache_until has passed and new data could not be got from CCP
         * (e.g. due to servers being down for downtime)
         */
        EXPIRED,
        /**Has no local data at all, and CCP servers were unavailable (e.g. downtime)*/
        FAILED,
        /**Had data, but it was purged*/
        PURGED
    };
    CrestCacheEntry()
        : status(NEW)
        , preloaded(false)
        , mutex()
        , update_wait()
        , path()
        , cache_until(0)
        , last_used(0)
        , data()
    {}
    ~CrestCacheEntry()
    {
        std::unique_lock<std::mutex> lock(mutex);
        if (status == UPDATING) update_wait.wait(lock);
    }

    Status status;
    /** Used as a flag to show this is a preloaded entry. Preloaded entries never get purged,
     *  since they would just immediately get reloaded regardless of usage.
     */
    bool preloaded;
    /**"row" level locking*/
    std::mutex mutex;
    /**For waiting for updates.*/
    std::condition_variable update_wait;
    /**The public-crest path that this cache entry is for.*/
    std::string path;
    /**When the cache expires.*/
    time_t cache_until;
    /**When the data was last used. Used to inform the cache purge policy.*/
    time_t last_used;
    /**The cached data (gzipped)*/
    std::vector<uint8_t> data;

    CrestHttpRequest http_request;

    void wait(std::unique_lock<std::mutex> &lock)
    {
        if (status == CrestCacheEntry::UPDATING)
        {
            update_wait.wait(lock);
        }
        if (status == FAILED)
        {
            throw CrestRequestFailed();
        }
    }

    bool is_data_valid()const
    {
        return status != NEW && status != FAILED && status != PURGED;
    }
    /**Has data within expiry*/
    bool is_current_data()const
    {
        return is_data_valid() && time(nullptr) < cache_until;
    }
};
