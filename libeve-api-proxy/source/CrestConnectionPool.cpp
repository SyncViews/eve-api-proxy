#include "Precompiled.hpp"
#include "CrestConnectionPool.hpp"
#include "CppServers.hpp"
#include "http/CrestRequest.hpp"
#include "http/core/HttpParser.hpp"
#include "Gzip.hpp"
#include <iostream>

CrestConnectionPool::CrestConnectionPool()
    : exiting(false), mutex(), request_queued(), request_queue(), connections()
    , request_allowance(0)
    , request_allowance_time(0)
{
    for (auto i = 0; i < PCREST_MAX_CONNECTIONS; ++i)
    {
        connections.emplace_back(this);
    }
}

CrestConnectionPool::~CrestConnectionPool()
{
    exit();
}

void CrestConnectionPool::exit()
{
    {
        std::unique_lock<std::mutex> lock(mutex);
        exiting = true;
        request_queued.notify_all();
    }
    for (auto &con : connections)
    {
        con.thread.join();
    }
    connections.clear();
}

void CrestConnectionPool::queue(CrestHttpRequest *request)
{
    assert(!exiting);
    {
        std::unique_lock<std::mutex> lock(mutex);
        request_queue.push(request);
        request_queued.notify_one();
    }
}

void CrestConnectionPool::wait_for_request_allowance()
{
    if (--request_allowance >= 0) return;

    std::unique_lock<std::mutex> lock(mutex);
    //only one thread actively waits to reset the count, the others will block on the mutex and
    //then continue using this second atomic int decrement
    if (--request_allowance >= 0) return;


    using std::chrono::steady_clock;
    using std::chrono::seconds;
    auto now = steady_clock::now();
    auto nows = std::chrono::duration_cast<seconds>(now.time_since_epoch());

    std::cout << "CREST request allowance used. Renewing. ";
    std::cout << " allowance_time: " << request_allowance_time.count();
    std::cout << " now: " << nows.count();
    std::cout << std::endl;

    
    if (request_allowance_time < nows)
    {
        request_allowance_time = nows;
    }
    else
    {
        request_allowance_time = nows + seconds(1);
        std::this_thread::sleep_until(steady_clock::time_point(request_allowance_time));

        nows = std::chrono::duration_cast<seconds>(steady_clock::now().time_since_epoch());
        std::cout << "CREST request allowance renewed at " << nows.count() << std::endl;
    }
    request_allowance = PCREST_MAX_REQS_PER_SEC - 1;
}


CrestConnectionPool::CrestConnection::CrestConnection(CrestConnectionPool *pool)
    : pool(pool), thread(), socket()
{
    thread = std::thread(std::bind(&CrestConnectionPool::CrestConnection::main, this));
}

CrestConnectionPool::CrestConnection::~CrestConnection()
{

}

void CrestConnectionPool::CrestConnection::main()
{
    set_thread_name("crest-pool");
    while (true)
    {
        CrestHttpRequest *request = nullptr;
        
        {
            std::unique_lock<std::mutex> lock(pool->mutex);
            while (true)
            {
                if (!pool->request_queue.empty())
                {
                    request = pool->request_queue.front();
                    pool->request_queue.pop();
                    break;
                }
                else if (pool->exiting)
                {
                    return;
                }
                else
                {
                    pool->request_queued.wait(lock);
                }
            }
        }
        assert(request);
        pool->wait_for_request_allowance();

        for (int i = 0; !process_request(request); +i)
        {
            std::this_thread::sleep_for(std::chrono::seconds(i));
        }
    }
}

bool CrestConnectionPool::CrestConnection::process_request(CrestHttpRequest * request)
{
    if (!socket.is_connected())
    {
        std::cout << "Connecting to " << PCREST_HOST << ":" << PCREST_PORT << std::endl;
        socket.connect(PCREST_HOST, PCREST_PORT);
    }
    //request
    send_crest_get_request(&socket, request->get_uri_path());
    //response
    http::HttpParser response(false);
    while (!response.is_completed())
    {
        uint8_t buffer[4096];
        auto len = socket.recv(buffer, sizeof(buffer));
        if (!len)
        {
            std::cerr << "Unexpected CREST disconnect, retry" << std::endl;
            socket.close();
            return false;
        }
        auto len2 = response.read(buffer, len);
        //TODO: With pipelining, will need to handle the case where also read part of the
        //next response
        assert(len == len2);
    }

    request->response.status_code = response.get_status_code();
    request->response.status_msg = response.get_status_message();
    if (response.get_headers().get("Content-Encoding") == "gzip")
    {
        request->response.body = gzip_decompress(response.get_body());
    }
    else
    {
        request->response.body = response.get_body();
    }
    if (response.get_headers().get("Connection") != "keep-alive")
    {
        std::cout << "Terminating public-crest connection because Connection: " << response.get_headers().get("Connection") << std::endl;
        socket.close();
    }

    request->promise.set_value(request);
    if (request->on_completion) request->on_completion(request);
    return true;
}
