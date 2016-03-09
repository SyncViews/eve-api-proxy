#include "Precompiled.hpp"
#include "CrestCache.hpp"

CrestCache::CrestCache()
    : cache_mutex(), cache(), preload_requests()
    , preload_request_next(0)
    , crest_connection_pool(this)
{
    std::unique_lock<std::mutex> lock(cache_mutex);
    std::ifstream is("preload_requests.txt");
    std::string line;
    while (std::getline(is, line))
    {
        preload_requests.push_back(line);
    }
    std::cout << "Got " << preload_requests.size() << " CREST requests to keep preloaded" << std::endl;
}

CrestCache::~CrestCache()
{
    stop();
}

void CrestCache::stop()
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


CrestHttpRequest *CrestCache::get_preload_request()
{
    std::unique_lock<std::mutex> lock(cache_mutex);
    if (preload_requests.empty()) return nullptr;
    size_t i = preload_request_next;
    do
    {
        auto &entry = get_entry(preload_requests[i]);
        std::unique_lock<std::mutex> entry_lock(entry.mutex);

        if (entry.status == CrestCacheEntry::FAILED ||
            (entry.status != CrestCacheEntry::UPDATING && time(nullptr) > entry.cache_until))
        {
            entry.status = CrestCacheEntry::UPDATING;
            entry.http_request = CrestHttpRequest(entry.path,
                std::bind(&CrestCache::update_entry_completion, this, &entry, std::placeholders::_1));
            std::cout << "Preloading " << entry.path << std::endl;
            return &entry.http_request;
        }

        i = (i + 1) % preload_requests.size();
    }
    while (i != preload_request_next);
    return nullptr;
}

CrestCacheEntry &CrestCache::get_entry(const std::string &path)
{
    auto &entry = cache[path];
    if (entry.path.empty())
    {
        entry.path = path;
    }
    return entry;
}

CrestCacheEntry *CrestCache::get_locked(const std::string &path, std::unique_lock<std::mutex> &entry_lock)
{
    std::unique_lock<std::mutex> lock(cache_mutex);
    auto &entry = get_entry(path);

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
