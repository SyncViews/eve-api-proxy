#include "Precompiled.hpp"
#include "CrestCache.hpp"

CrestCache::CrestCache()
    : cache_mutex(), cache_size(0), cache(), preload_entries()
    , preload_request_next(0)
    , crest_connection_pool(this)
{
    std::unique_lock<std::mutex> lock(cache_mutex);
    std::ifstream is("preload_requests.txt");
    std::string line;
    while (std::getline(is, line))
    {
        if (!line.empty() && line[0] != '#')
        {
            auto entry = &cache[line];
            entry->path = line;
            entry->preloaded = true;
            preload_entries.push_back(entry);
        }
    }
    log_info() << "Got " << preload_entries.size() << " CREST requests to keep preloaded" << std::endl;
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
    if (preload_entries.empty()) return nullptr;
    size_t i = preload_request_next;
    do
    {
        auto entry = preload_entries[i];
        std::unique_lock<std::mutex> entry_lock(entry->mutex);

        if (entry->status != CrestCacheEntry::UPDATING && !entry->is_current_data())
        {
            entry->status = CrestCacheEntry::UPDATING;
            entry->http_request = CrestHttpRequest(entry->path,
                std::bind(&CrestCache::update_entry_completion, this, entry, std::placeholders::_1));
            log_debug() << "Preloading " << entry->path << std::endl;
            return &entry->http_request;
        }

        i = (i + 1) % preload_entries.size();
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
    entry.last_used = time(nullptr);
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
    std::unique_lock<std::mutex> cache_lock(cache_mutex);
    {
        std::unique_lock<std::mutex> entry_lock(entry->mutex);
        auto response = request->get_response();
        if (response.status_code == 200)
        {
            if (!entry->data.empty())
            {
                assert(cache_size >= entry->data.size());
                if (cache_size < entry->data.size()) cache_size = 0; //just to be safe
                else cache_size -= entry->data.size();
            }
            cache_size += response.body.size();
        
            entry->data = response.body;
            entry->last_used = time(nullptr);
            entry->cache_until = entry->last_used + 300; //TODO: take this from the response
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
    check_cache_purge();
}

void CrestCache::check_cache_purge()
{
    if (cache_size <= MAX_CACHE_SIZE) return;
    
    //Purge data to get under 90% of limit
    size_t must_purge = (MAX_CACHE_SIZE * 10 / 100) + (cache_size - MAX_CACHE_SIZE);
    size_t purged = 0;

    typedef std::pair<unique_lock,  CrestCacheEntry*> LockedEntry;
    std::vector<LockedEntry> candidates;
    // Check cache contents
    bool expired_only = true;
    for (auto &i : cache)
    {
        auto &entry = i.second;
        unique_lock lock(entry.mutex, std::try_to_lock);
        if (lock && !entry.preloaded)// if couldnt get the lock, its active so would not want to purge anyway
        {
            if (!entry.is_current_data())
            { // Expired, so just purge it now
                purged += entry.data.size();
                entry.data.clear();
                entry.status = CrestCacheEntry::PURGED;
            }
            else if (!entry.data.empty())
            { // Valid, consider for purging
                candidates.emplace_back(std::move(lock), &entry);
            }
        }
    }
    if (purged < must_purge)
    {
        // Did not purge enough expired entries, so start with the least recently used ones
        std::sort(candidates.begin(), candidates.end(),
            [](const LockedEntry &a, const LockedEntry &b) {
                return a.second->last_used < b.second->last_used; });
        for (auto i = candidates.begin(); i != candidates.end() && purged < must_purge; ++i)
        {
            expired_only = false;
            purged += i->second->data.size();
            i->second->data.clear();
            i->second->status = CrestCacheEntry::PURGED;
        }
    }
    assert(purged <= cache_size);
    if (purged > cache_size) cache_size = 0; //just in case
    else cache_size -= purged;
    std::cout << "Purged " << (purged / 1024.0 / 1024.0) << " MB from the cache" << std::endl;
}
