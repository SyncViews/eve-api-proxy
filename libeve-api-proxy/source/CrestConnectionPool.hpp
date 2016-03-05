#pragma once
#include "socket/TlsSocket.hpp"
#include "CrestHttpRequest.hpp"
#include <atomic>
#include <mutex>
#include <thread>
#include <list>
#include <queue>
#include <condition_variable>

class CrestConnectionPool
{
public:
    CrestConnectionPool();
    ~CrestConnectionPool();
    void exit();
    void queue(CrestHttpRequest *request);

private:
    struct CrestConnection
    {
        CrestConnection(CrestConnectionPool *pool);
        ~CrestConnection();

        void main();
        void process_request(CrestHttpRequest *request);
        
        CrestConnectionPool *pool;
        std::thread thread;
        TlsSocket socket;
    };
    std::atomic<bool> exiting;
    std::mutex mutex;
    std::condition_variable request_queued;
    std::queue<CrestHttpRequest *> request_queue;
    std::list<CrestConnection> connections;
    
};
