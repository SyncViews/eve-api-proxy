#pragma once
#include <memory>
#include <queue>
#include "HttpRequest.hpp"
#include "HttpResponseParser.hpp"
#include "socket/Socket.hpp"
namespace http
{
    //TODO: This is going to want non-blocking/async sockets implemented because otherwise
    //going to end up blocked by a recv on the first request when should be sending or preparing
    //the next one...

    /**@brief Controls a single HTTP connection to process requests and responses.
     * Is able to send multiple requests, including multiple in-progress (HTTP
     * pipelining) and handles the Connection headers.
     * 
     * Note does not handle request retry if a request that started failed, in which case the
     * response object will be marked as completed, but will not contain a valid HTTP response.
     */
    class HttpClientPipeline
    {
    public:
        static const size_t MAX_REQUESTS_INFLIGHT = 1;
        /**Queues a request. The caller must ensure the request object remains valid until it
         * is completed.
         */
        void queue_request(HttpRequest *request);
        /**Attempts to unqueue the request if it has not yet started.*/
        bool cancel_request(HttpRequest *request);
        /**Returns a completed request. Use get_response and finish_response.*/
        HttpRequest *completed_request();
        /**Get the response for completed_request*/
        const HttpResponseParser &get_response();
        /**Finish with the response for completed_request so can start on the next one.*/
        void finish_response();

    private:
        std::unique_ptr<Socket> socket;
        std::queue<HttpRequest*> queued_requests;
        std::queue<HttpRequest*> inprogress_requests;
        HttpResponseParser response_parser;
    };
}
