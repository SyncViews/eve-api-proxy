#pragma once
#include "CacheEntry.hpp"
#include "ConnectionPool.hpp"
#include <string>
#include <unordered_map>
#include <mutex>

namespace crest
{
    class CacheOld
    {
    public:
        struct CacheLookupResults
        {
            CacheEntry *entry;
            /**Lock on entry->mutex*/
            std::unique_lock<std::mutex> lock;
        };
        struct CacheLookupFutureResults
        {
            CacheEntry *entry;
            CacheEntry::Status status;
            std::vector<uint8_t> data;
            void wait()
            {
                std::unique_lock<std::mutex> lock(entry->mutex);
                entry->wait(lock);
                status = entry->status;
                data = entry->data;
            }
        };

        CacheOld(ConnectionPool &crest_connection_pool);
        ~CacheOld();
        void stop();

        CacheEntry *get(const std::string &path);
        /**Get CREST data.
        * @param path The path to get from CREST.
        */
        CacheLookupFutureResults get_future(const std::string &path);
        CacheLookupResults get_now(const std::string &path);
    private:
        typedef std::unique_lock<std::mutex> unique_lock;
        typedef std::unordered_map<std::string, CacheEntry> CacheMap;
        static const size_t MAX_CACHE_SIZE = 1024 * 1024 * 20;//20MB

        /**Lock for the cache map.
         * @warning Never attempt to lock this if already holding any entry lock. Always lock this
         * before locking an entry. Failure to do so may result in dead lock.
         */
        std::mutex cache_mutex;
        size_t cache_size;
        CacheMap cache;
        /**CrestConnectionPool for updating cache entries.*/
        std::vector<CacheEntry*> preload_entries;
        size_t preload_request_next;
        ConnectionPool &crest_connection_pool;

        CacheEntry &get_entry(const std::string &path);
        CacheEntry *get_locked(const std::string &path, std::unique_lock<std::mutex> &lock);
        void update_entry(CacheEntry &entry);
        void update_entry_completion(CacheEntry *entry, http::Response *response);
        /**With the cache already locked, see if any data must be purged to remaining within
        * cache size limits.
        */
        void check_cache_purge();
    };
}
