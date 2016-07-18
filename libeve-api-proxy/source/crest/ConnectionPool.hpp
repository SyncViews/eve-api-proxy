#pragma once
#include "socket/TlsSocket.hpp"
#include "Request.hpp"
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
        void queue(Request *request);

    private:
        struct Connection
        {
            Connection(ConnectionPool *pool);
            ~Connection();

            void main();
            void process_request(Request *request);

            ConnectionPool *pool;
            std::thread thread;
            TlsSocket socket;
        };
        std::atomic<bool> exiting;
        /**Mutex to protect access to request_queue and connections.*/
        std::mutex mutex;
        std::condition_variable request_queued;
        /**First-in first-out queue of requests to execute.*/
        std::queue<Request *> request_queue;
        /**The pool of active connections.*/
        std::list<Connection> connections;

        /**Counter for the number of remaining requests in the current allocation.
         * See wait_for_request_allowance.
         */
        std::atomic<int> request_allowance;
        /**Time the allowance was allocated. See wait_for_request_allowance.*/
        std::chrono::seconds request_allowance_time;

        /**If attempting to fullfill a request fails multiple times, set tot the current time.
         * 
         */
        std::atomic<time_t> crest_unavailable_at;

        /**Used by threads to wait until they are allowed to execute a request within the rate-limit.
         * May block upto 1 second waiting for the new request allowance.
         * 
         * Any requests that are attempted within the next 10 seconds will be immediately failed.
         */
        void wait_for_request_allowance();
    };
}
