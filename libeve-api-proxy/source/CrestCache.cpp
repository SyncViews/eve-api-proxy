#include "Precompiled.hpp"
#include "CrestCache.hpp"

CrestCache::CrestCache()
{
}

CrestCache::~CrestCache()
{
    crest_connection_pool.exit();
}

CrestCacheEntry *CrestCache::get(const std::string &path)
{
    std::unique_lock<std::mutex> entry_lock;
    return get_locked(path, entry_lock);
}

CrestCache::CacheLookupFutureResults CrestCache::get_future(const std::string &path)
{
    std::unique_lock<std::mutex> entry_lock;
    auto entry = get_locked(path, entry_lock);
    return { entry, CrestCacheEntry::NEW, {}};
}

CrestCache::CacheLookupResults CrestCache::get_now(const std::string &path)
{
    std::unique_lock<std::mutex> entry_lock;
    auto entry = get_locked(path, entry_lock);
    entry->wait(entry_lock);
    return { entry, std::move(entry_lock) };
}

CrestCacheEntry *CrestCache::get_locked(const std::string &path, std::unique_lock<std::mutex> &entry_lock)
{
    std::unique_lock<std::mutex> lock(cache_mutex);
    auto &entry = cache[path];
    if (entry.path.empty())
    {
        entry.path = path;
    }

    entry_lock = std::unique_lock<std::mutex>(entry.mutex);
    lock.unlock();
    //no cache lock here, but the entry is locked

    if (entry.status == CrestCacheEntry::FAILED)
    {
        update_entry(entry);
    }
    else if (entry.status != CrestCacheEntry::UPDATING && time(nullptr) > entry.cache_until)
    {
        update_entry(entry);
    }
    else if (entry.status == CrestCacheEntry::UPDATED)
    {
        entry.status = CrestCacheEntry::HIT;
    }

    return &entry;
}

void CrestCache::update_entry(CrestCacheEntry &entry)
{
    entry.status = CrestCacheEntry::UPDATING;
    entry.http_request = CrestHttpRequest(entry.path,
        std::bind(&CrestCache::update_entry_completion, this, &entry, std::placeholders::_1));
    crest_connection_pool.queue(&entry.http_request);
}

void CrestCache::update_entry_completion(CrestCacheEntry *entry, CrestHttpRequest *request)
{
    std::unique_lock<std::mutex> lock(entry->mutex);
    auto response = request->get_response();
    if (response.status_code == 200)
    {
        entry->data = response.body;
        entry->cache_until = time(nullptr) + 300; //TODO: take this from the response
        entry->status = CrestCacheEntry::UPDATED;
    }
    else if (entry->cache_until)
    {
        entry->status = CrestCacheEntry::EXPIRED;
    }
    else
    {
        entry->status = CrestCacheEntry::FAILED;
    }
    entry->update_wait.notify_all();
}
