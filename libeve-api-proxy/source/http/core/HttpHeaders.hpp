#pragma once
#include "HttpHeader.hpp"
#include <string>
#include <vector>
#include <algorithm>
namespace http
{
    class HttpHeaders
    {
    public:
        std::vector<HttpHeader> headers;

        HttpHeader *find(const std::string &name)
        {
            auto it = std::find_if(headers.begin(), headers.end(), [&name](const HttpHeader &x) { return x.name == name; });
            return it != headers.end() ? &*it : nullptr;
        }
        const HttpHeader *find(const std::string &name) const
        {
            auto it = std::find_if(headers.begin(), headers.end(), [&name](const HttpHeader &x) { return x.name == name; });
            return it != headers.end() ? &*it : nullptr;
        }

        std::string get(const std::string &name)const
        {
            auto header = find(name);
            return header ? header->value : std::string();
        }

        void set(const std::string &name, const std::string &value)
        {
            auto header = find(name);
            if (header) header->value = value;
            else add(name, value);
        }
        void add(const std::string &name, const std::string &value)
        {
            headers.emplace_back(name, value);
        }
        void set_default(const std::string &name, const std::string &value)
        {
            auto header = find(name);
            if (!header) add(name, value);
        }
    };
}
