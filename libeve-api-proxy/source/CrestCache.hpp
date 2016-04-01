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
    struct CacheLookupFutureResults
    {
        CrestCacheEntry *entry;
        CrestCacheEntry::Status status;
        std::vector<uint8_t> data;
        void wait()
        {
            std::unique_lock<std::mutex> lock(entry->mutex);
            entry->wait(lock);
            status = entry->status;
            data = entry->data;
        }
    };
    
    CrestCache();
    ~CrestCache();
    void stop();

    CrestCacheEntry *get(const std::string &path);
    /**Get CREST data.
     * @param path The path to get from CREST.
     */
    CacheLookupFutureResults get_future(const std::string &path);
    CacheLookupResults get_now(const std::string &path);

    /**Used by CrestConnectionPool to use up spare bandwidth by getting requests to cache in
     * advance.
     */
    CrestHttpRequest *get_preload_request();
private:
    typedef std::unique_lock<std::mutex> unique_lock;
    typedef std::unordered_map<std::string, CrestCacheEntry> Cache;
    static const size_t MAX_CACHE_SIZE = 1024*1024*20//20MB

    /**Lock for the cache map.
     * @warning Never attempt to lock this if already holding any entry lock. Always lock this
     * before locking an entry. Failure to do so may result in dead lock.
     */
    std::mutex cache_mutex;
    size_t cache_size;
    Cache cache;
    /**CrestConnectionPool for updating cache entries.*/
    std::vector<CrestCacheEntry*> preload_entries;
    size_t preload_request_next;
    CrestConnectionPool crest_connection_pool;

    CrestCacheEntry &get_entry(const std::string &path);
    CrestCacheEntry *get_locked(const std::string &path, std::unique_lock<std::mutex> &lock);
    void update_entry(CrestCacheEntry &entry);
    void update_entry_completion(CrestCacheEntry *entry, CrestHttpRequest *request);
    /**With the cache already locked, see if any data must be purged to remaining within
     * cache size limits.
     */
    void check_cache_purge();
};
