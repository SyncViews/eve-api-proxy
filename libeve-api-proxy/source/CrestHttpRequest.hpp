#pragma once
#include <future>
#include <string>
class CrestHttpRequest
{
public:
    struct Response
    {
        int status_code;
        std::string status_msg;
        std::vector<uint8_t> body;
    };
    explicit CrestHttpRequest(const std::string &uri_path)
        : promise(), uri_path(uri_path), response()
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
    friend class CrestConnectionPool;
    std::promise<CrestHttpRequest*> promise;
    std::string uri_path;
    Response response;
};
