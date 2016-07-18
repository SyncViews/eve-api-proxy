#include "Precompiled.hpp"
#include "ConnectionPool.hpp"
#include "Error.hpp"
#include "Cache.hpp"
#include "CppServers.hpp"
#include "http/CrestRequest.hpp"
#include "http/core/HttpParser.hpp"
#include "Gzip.hpp"
#include <iostream>
namespace crest
{

    ConnectionPool::ConnectionPool()
        : exiting(false), mutex(), request_queued(), request_queue(), connections()
        , request_allowance(0)
        , request_allowance_time(0)
        , crest_unavailable_at(0)
    {
        for (auto i = 0; i < PCREST_MAX_CONNECTIONS; ++i)
        {
            connections.emplace_back(this);
        }
    }

    ConnectionPool::~ConnectionPool()
    {
        exit();
    }

    void ConnectionPool::exit()
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

    void ConnectionPool::queue(Request *request)
    {
        assert(!exiting);
        {
            std::unique_lock<std::mutex> lock(mutex);
            request_queue.push(request);
            request_queued.notify_one();
        }
    }

    void ConnectionPool::wait_for_request_allowance()
    {
        //atomic decrement, no need to lock if not managing the reset
        if (--request_allowance >= 0) return;
        //only one thread actively waits to reset the count, the others will block on the mutex and
        //then continue using this second int decrement
        std::unique_lock<std::mutex> lock(mutex);
        if (--request_allowance >= 0) return;

        //Make the thread sleep until the second after the previous allowance timestamp
        using std::chrono::steady_clock;
        using std::chrono::seconds;
        auto now = steady_clock::now();
        auto nows = std::chrono::duration_cast<seconds>(now.time_since_epoch());

        if (request_allowance_time < nows)
        {   // More than a second passed since the last allowance anyway
            request_allowance_time = nows;
        }
        else
        {
            std::this_thread::sleep_until(steady_clock::time_point(request_allowance_time));
            request_allowance_time += seconds(1);
        }
        request_allowance = PCREST_MAX_REQS_PER_SEC - 1;
        log_info() << "CREST request allowance renewed at " << request_allowance_time.count() << std::endl;
    }


    ConnectionPool::Connection::Connection(ConnectionPool *pool)
        : pool(pool), thread(), socket()
    {
        thread = std::thread(std::bind(&ConnectionPool::Connection::main, this));
    }

    ConnectionPool::Connection::~Connection()
    {

    }

    void ConnectionPool::Connection::main()
    {
        set_thread_name("crest-pool");
        while (true)
        {
            Request *request = nullptr;
            bool crest_available;

            {
                std::unique_lock<std::mutex> lock(pool->mutex);
                while (true)
                {
                    auto now = time(nullptr);
                    crest_available = pool->crest_unavailable_at + 10 < now;
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
            if (!crest_available)
            {
                // If CREST does not appear to be available, fail immediately so as to not block the request
                // (e.g. a live web page), and to avoid making hundreds of failed connect attempts per second
                log_warning() << "Failed CREST request because believe CREST is down" << std::endl;
                request->response.http_success = false;
                request->promise.set_value(request);
                if (request->on_completion) request->on_completion(request);
                continue;
            }

            assert(request);
            assert(crest_available);
            pool->wait_for_request_allowance();

            for (int i = 0;; ++i)
            {
                try
                {
                    process_request(request);
                    break;
                }
                catch (const NetworkError &e)
                {
                    log_error() << "CREST request failed with: " << e.what() << std::endl;
                    try
                    {
                        if (socket.is_connected()) socket.force_close();
                    }
                    catch (const NetworkError &e)
                    {
                        log_error() << "Closing CREST socket following failure also failed: " << e.what() << std::endl;
                        socket.force_close();
                    }
                    if (i >= 2)
                    {
                        log_error() << "Aborted CREST request after communication errors" << std::endl;
                        pool->crest_unavailable_at = time(nullptr);
                        request->response.http_success = false;
                        request->promise.set_value(request);
                        if (request->on_completion) request->on_completion(request);
                        break;
                    }
                }
                std::this_thread::sleep_for(std::chrono::seconds(i));
            }
        }
    }

    void ConnectionPool::Connection::process_request(Request * request)
    {
        if (!socket.is_connected())
        {
            log_debug() << "Connecting to " << PCREST_HOST << ":" << PCREST_PORT << std::endl;
            socket.connect(PCREST_HOST, PCREST_PORT);
            log_debug() << "Connected to public-crest" << std::endl;
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
                throw NetworkError("Unexpected CREST disconnect while reading HTTP response");
            }
            auto len2 = response.read(buffer, len);
            //TODO: With pipelining, will need to handle the case where also read part of the
            //next response
            assert(len == len2);
        }

        request->response.http_success = true;
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
            log_warning() << "Terminating public-crest connection because Connection: " << response.get_headers().get("Connection") << std::endl;
            socket.close();
        }

        request->promise.set_value(request);
        if (request->on_completion) request->on_completion(request);
    }
}
