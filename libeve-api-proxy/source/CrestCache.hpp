#pragma once
#include "CrestCacheEntry.hpp"
#include "CrestConnectionPool.hpp"
#include <string>
#include <unordered_map>
#include <mutex>

class CrestCache
{
public:
    struct CacheLookupResults
    {
        CrestCacheEntry *entry;
        /**Lock on entry->mutex*/
        std::unique_lock<std::mutex> lock;
    };
    
    CrestCache();
    ~CrestCache();
    
    /**Get CREST data.
     * @param path The path to get from CREST.
     */
    CacheLookupResults get(const std::string &path);

private:
    std::mutex cache_mutex;
    std::unordered_map<std::string, CrestCacheEntry> cache;
    /**CrestConnectionPool for updating cache entries.*/
    CrestConnectionPool crest_connection_pool;

    void update_entry(CrestCacheEntry &entry);
};
