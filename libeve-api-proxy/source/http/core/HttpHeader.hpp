#pragma once
#include <string>
namespace http
{
    class HttpHeader
    {
    public:
        std::string name;
        std::string value;

        HttpHeader(const std::string &name, const std::string &value)
            : name(name), value(value)
        {}
        HttpHeader(std::string &&name, std::string &&value)
            : name(std::move(name)), value(std::move(value))
        {}
        HttpHeader(HttpHeader &&mv)
            : name(std::move(mv.name)), value(std::move(mv.value))
        {}
        HttpHeader(const HttpHeader &cp)
            : name(cp.name), value(cp.value)
        {}
    };
}
