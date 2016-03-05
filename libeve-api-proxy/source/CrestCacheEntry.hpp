#pragma once
#include <condition_variable>
#include <ctime>
#include <mutex>
#include <string>
#include <vector>
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
        FAILED
    };
    CrestCacheEntry()
        : status(NEW)
        , mutex()
        , update_wait()
        , path()
        , cache_until(0)
        , data()
    {}
    ~CrestCacheEntry()
    {
        std::unique_lock<std::mutex> lock(mutex);
        if (status == UPDATING) update_wait.wait(lock);
    }
    Status status;
    /**"row" level locking*/
    std::mutex mutex;
    /**For waiting for updates.*/
    std::condition_variable update_wait;
    /**The public-crest path that this cache entry is for.*/
    std::string path;
    /**When the cache expires.*/
    time_t cache_until;
    /**The cached data (gzipped)*/
    std::vector<uint8_t> data;
};
