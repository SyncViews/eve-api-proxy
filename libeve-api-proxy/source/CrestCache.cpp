#include "Precompiled.hpp"
#include "CrestCache.hpp"

CrestCache::CrestCache()
{
}

CrestCache::~CrestCache()
{
    crest_connection_pool.exit();
}

CrestCache::CacheLookupResults CrestCache::get(const std::string &path)
{
    std::unique_lock<std::mutex> lock(cache_mutex);
    auto &entry = cache[path];
    if (entry.path.empty())
    {
        entry.path = path;
    }

    std::unique_lock<std::mutex> entry_lock(entry.mutex);
    lock.unlock();
    //no cache lock here, but the entry is locked

    if (entry.status == CrestCacheEntry::UPDATING) entry.update_wait.wait(entry_lock);

    if (entry.status == CrestCacheEntry::FAILED || time(nullptr) > entry.cache_until)
    {
        update_entry(entry);
    }
    else if (entry.status == CrestCacheEntry::UPDATED)
    {
        entry.status = CrestCacheEntry::HIT;
    }
    
    return {&entry, std::move(entry_lock)};
}

void CrestCache::update_entry(CrestCacheEntry &entry)
{
    CrestHttpRequest request(entry.path);
    crest_connection_pool.queue(&request);
    auto response = request.wait();
    if (response->status_code == 200)
    {
        entry.data = response->body;
        entry.cache_until = time(nullptr) + 300; //TODO: take this from the response
        entry.status = CrestCacheEntry::UPDATED;
    }
    else if (entry.cache_until)
    {
        entry.status = CrestCacheEntry::EXPIRED;
    }
    else
    {
        entry.status = CrestCacheEntry::FAILED;
    }
    entry.update_wait.notify_all();
}
