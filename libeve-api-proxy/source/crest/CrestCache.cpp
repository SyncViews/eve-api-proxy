#include "Precompiled.hpp"
#include "Cache.hpp"

namespace crest
{
    Cache::Cache()
        : cache_mutex(), cache_size(0), cache(), preload_entries()
        , preload_request_next(0)
        , crest_connection_pool()
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

    Cache::~Cache()
    {
        stop();
    }

    void Cache::stop()
    {
        crest_connection_pool.exit();
    }

    CacheEntry *Cache::get(const std::string &path)
    {
        std::unique_lock<std::mutex> entry_lock;
        return get_locked(path, entry_lock);
    }

    Cache::CacheLookupFutureResults Cache::get_future(const std::string &path)
    {
        std::unique_lock<std::mutex> entry_lock;
        auto entry = get_locked(path, entry_lock);
        return{ entry, CacheEntry::NEW,{} };
    }

    Cache::CacheLookupResults Cache::get_now(const std::string &path)
    {
        std::unique_lock<std::mutex> entry_lock;
        auto entry = get_locked(path, entry_lock);
        entry->wait(entry_lock);
        return{ entry, std::move(entry_lock) };
    }


    Request *Cache::get_preload_request()
    {
        std::unique_lock<std::mutex> lock(cache_mutex);
        if (preload_entries.empty()) return nullptr;
        size_t i = preload_request_next;
        do
        {
            auto entry = preload_entries[i];
            std::unique_lock<std::mutex> entry_lock(entry->mutex);

            if (entry->status != CacheEntry::UPDATING && !entry->is_current_data())
            {
                entry->status = CacheEntry::UPDATING;
                entry->http_request = Request(entry->path,
                    std::bind(&Cache::update_entry_completion, this, entry, std::placeholders::_1));
                log_debug() << "Preloading " << entry->path << std::endl;
                return &entry->http_request;
            }

            i = (i + 1) % preload_entries.size();
        } while (i != preload_request_next);
        return nullptr;
    }

    CacheEntry &Cache::get_entry(const std::string &path)
    {
        auto &entry = cache[path];
        if (entry.path.empty())
        {
            entry.path = path;
        }
        return entry;
    }

    CacheEntry *Cache::get_locked(const std::string &path, std::unique_lock<std::mutex> &entry_lock)
    {
        std::unique_lock<std::mutex> lock(cache_mutex);
        auto &entry = get_entry(path);

        entry_lock = std::unique_lock<std::mutex>(entry.mutex);
        lock.unlock();
        //no cache lock here, but the entry is locked

        if (entry.status == CacheEntry::FAILED || entry.status == CacheEntry::PURGED)
        {
            update_entry(entry);
        }
        else if (entry.status != CacheEntry::UPDATING && time(nullptr) > entry.cache_until)
        {
            update_entry(entry);
        }
        else if (entry.status == CacheEntry::UPDATED)
        {
            entry.status = CacheEntry::HIT;
        }
        entry.last_used = time(nullptr);
        return &entry;
    }

    void Cache::update_entry(CacheEntry &entry)
    {
        entry.status = CacheEntry::UPDATING;
        entry.http_request = Request(entry.path,
            std::bind(&Cache::update_entry_completion, this, &entry, std::placeholders::_1));
        crest_connection_pool.queue(&entry.http_request);
    }

    void Cache::update_entry_completion(CacheEntry *entry, Request *request)
    {
        std::unique_lock<std::mutex> cache_lock(cache_mutex);
        {
            std::unique_lock<std::mutex> entry_lock(entry->mutex);
            auto response = request->get_response();
            if (!response.http_success)
            {
                entry->status = CacheEntry::FAILED;
            }
            else if (response.status_code == 200)
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
                entry->status = CacheEntry::UPDATED;
            }
            else if (entry->cache_until)
            {
                entry->status = CacheEntry::EXPIRED;
            }
            else
            {
                entry->status = CacheEntry::FAILED;
            }
            entry->update_wait.notify_all();
        }
        check_cache_purge();
    }

    void Cache::check_cache_purge()
    {
        if (cache_size <= MAX_CACHE_SIZE) return;

        //Purge data to get under 90% of limit
        size_t must_purge = (MAX_CACHE_SIZE * 10 / 100) + (cache_size - MAX_CACHE_SIZE);
        size_t purged = 0;

        typedef std::pair<unique_lock, CacheEntry*> LockedEntry;
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
                    entry.data = std::vector<uint8_t>();
                    assert(entry.data.capacity() == 0);
                    entry.status = CacheEntry::PURGED;
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
                i->second->data = std::vector<uint8_t>();
                assert(i->second->data.capacity() == 0);
                i->second->status = CacheEntry::PURGED;
            }
        }
        assert(purged <= cache_size);
        if (purged > cache_size) cache_size = 0; //just in case
        else cache_size -= purged;
        log_info() << "Purged " << (purged / 1024.0 / 1024.0) << " MB from the cache" << std::endl;
    }
}
