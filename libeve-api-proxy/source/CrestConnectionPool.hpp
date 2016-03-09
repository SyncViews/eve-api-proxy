#pragma once
#include "socket/TlsSocket.hpp"
#include "CrestHttpRequest.hpp"
#include <atomic>
#include <mutex>
#include <thread>
#include <list>
#include <queue>
#include <condition_variable>

class CrestCache;
class CrestConnectionPool
{
public:
    CrestConnectionPool(CrestCache *cache = nullptr);
    ~CrestConnectionPool();
    void exit();
    void queue(CrestHttpRequest *request);

private:
    struct CrestConnection
    {
        CrestConnection(CrestConnectionPool *pool);
        ~CrestConnection();

        void main();
        bool process_request(CrestHttpRequest *request);
        
        CrestConnectionPool *pool;
        std::thread thread;
        TlsSocket socket;
    };
    CrestCache *cache;
    std::atomic<bool> exiting;
    std::mutex mutex;
    std::condition_variable request_queued;
    std::queue<CrestHttpRequest *> request_queue;
    std::list<CrestConnection> connections;
    
    std::atomic<int> request_allowance;
    std::chrono::seconds request_allowance_time;

    /**Used by threads to wait until they are allowed to execute a request within the rate-limit.
     * May block upto 1 second waiting for the new request allowance.
     */
    void wait_for_request_allowance();
};
