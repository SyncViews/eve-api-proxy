#include "Precompiled.hpp"
#include "CrestConnectionPool.hpp"
#include "CppServers.hpp"
#include "http/CrestRequest.hpp"
#include "http/core/HttpResponseParser.hpp"
#include "Gzip.hpp"

CrestConnectionPool::CrestConnectionPool()
    : exiting(false), mutex(), request_queued(), request_queue(), connections()
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
        process_request(request);
    }
}

void CrestConnectionPool::CrestConnection::process_request(CrestHttpRequest * request)
{
    if (!socket.is_connected())
    {
        socket.connect(PCREST_HOST, PCREST_PORT);
    }
    //request
    send_crest_get_request(&socket, request->get_uri_path());
    //response
    http::HttpResponseParser response;
    while (!response.is_completed())
    {
        uint8_t buffer[4096];
        auto len = socket.recv(buffer, sizeof(buffer));
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
        socket.close();
    }

    request->promise.set_value(request);
    if (request->on_completion) request->on_completion(request);
}
