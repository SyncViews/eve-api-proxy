#pragma once
#include <string>
#include <stdexcept>
#include "core/HttpStatusCode.hpp"
namespace http
{

    class HttpStatusError : public std::runtime_error
    {
    public:
        HttpStatusError(int code, const std::string &msg)
            : std::runtime_error(
                "HTTP " + std::to_string(code) + " " + get_status_code_str(code) + ": " + msg)
            , code(code)
            , msg(msg)
        {}

        int status_code()const { return code; }
        const std::string &message()const { return msg; }
    private:
        int code;
        std::string msg;
    };

    class HttpNotFound : public HttpStatusError
    {
    public:
        explicit HttpNotFound(const std::string &msg) : HttpStatusError(404, msg) {}
    };

    class HttpBadRequest : public HttpStatusError
    {
    public:
        explicit HttpBadRequest(const std::string &msg) : HttpStatusError(400, msg) {}
    };

    class HttpInternalServerError : public HttpStatusError
    {
    public:
        explicit HttpInternalServerError(const std::string &msg) : HttpStatusError(500, msg) {}
    };
}
