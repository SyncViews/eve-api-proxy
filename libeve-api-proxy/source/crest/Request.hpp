#pragma once
#include <functional>
#include <future>
#include <string>
class Socket;
namespace crest
{
    /**A request for the crest::ConnectionPool to execute.*/
    class Request
    {
    public:
        typedef std::function<void(Request*)> CompletionFunc;
        struct Response
        {
            int status_code;
            std::string status_msg;
            std::vector<uint8_t> body;
            /**The HTTP request completed, does not imply the HTTP status code was 200*/
            bool http_success;
        };
        Request() {}
        explicit Request(const std::string &uri_path, CompletionFunc on_completion = nullptr)
            : promise(), on_completion(on_completion), uri_path(uri_path), response()
        {
        }

        const std::string &get_uri_path()const { return uri_path; }
        const Response &get_response()const { return response; }
        Response *wait()
        {
            promise.get_future().wait();
            return &response;
        }
    private:
        friend class ConnectionPool;
        std::promise<Request*> promise;
        CompletionFunc on_completion;
        std::string uri_path;
        Response response;
    };
    void send_get_request(Socket *socket, const std::string &path);
}
