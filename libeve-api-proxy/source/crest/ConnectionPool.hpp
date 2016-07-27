#pragma once
#include <http/client/SocketFactory.hpp>
#include <http/client/AsyncClient.hpp>
#include <http/net/Socket.hpp>
#include <atomic>
#include <mutex>
#include <thread>
#include <list>
#include <queue>
#include <condition_variable>

namespace crest
{
    /**@brief Pool of connections to execute CREST requests with rate-limiter.*/
    class ConnectionPool
    {
    public:
        ConnectionPool();
        ~ConnectionPool();
        /**Terminate all workers once the current requests are complete.*/
        void exit();
        /**Asynchronously queue a request to send.*/
        void queue(http::AsyncRequest *request);

    private:
        http::DefaultSocketFactory socket_factory;
        http::AsyncClient http_client;
    };
}
