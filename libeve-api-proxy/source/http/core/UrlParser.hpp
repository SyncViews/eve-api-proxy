#pragma once
#include <string>
#include <vector>
#include <map>
namespace http
{
    class UrlParser
    {
    public:
        std::string url;

        std::string protocol;
        std::string host;
        std::string port;
        std::string path;
        std::multimap<std::string, std::string> query_params;

        explicit UrlParser(const std::string &url)
            : url(url)
        {
            //  [<protocol>://<host>[:<port>]<path>[?<query>]
            size_t p = 0;

            auto proto_end = url.find("://", p);
            if (proto_end != std::string::npos)
            {
                protocol = url.substr(0, proto_end);
                p = proto_end + 3;

                auto host_end = url.find_first_of(":/", p);
                if (host_end == std::string::npos) throw std::runtime_error("Invalid url");
                host = url.substr(p, host_end - p);
                p = host_end;

                if (url[p] == ':')
                {
                    ++p; //skip the ':'
                    auto port_end = url.find('/', p);
                    if (port_end == std::string::npos) throw std::runtime_error("Invalid url");
                    port = url.substr(p, port_end - p);
                    p = port_end;
                }
            }
            
            if (url[p] != '/') throw std::runtime_error("Invalid url");
            
            auto path_end = url.find('?', p);
            if (path_end == std::string::npos)
            {
                path = url.substr(p);
            }
            else
            {
                path = url.substr(p, path_end - p);
                p = path_end + 1;
                while (true)
                {
                    auto param_key_end = url.find_first_of("=&", p);
                    if (param_key_end == std::string::npos)
                    {
                        query_params.emplace(url.substr(p), std::string());
                        break;
                    }
                    else if (url[param_key_end] == '&')
                    {
                        query_params.emplace(url.substr(p, param_key_end - p), std::string());
                        p = param_key_end + 1;
                    }
                    else
                    {
                        auto key = url.substr(p, param_key_end - p);
                        p = param_key_end + 1;
                        auto param_value_end = url.find('&', p);
                        if (param_value_end == std::string::npos)
                        {
                            query_params.emplace(key, url.substr(p));
                            break;
                        }
                        else
                        {
                            query_params.emplace(key, url.substr(p, param_value_end- p));
                            p = param_value_end + 1;
                        }
                    }
                }
            }
        }

        bool has_query_param(const std::string &key)
        {
            return query_params.find(key) != query_params.end();
        }

        std::string query_param(const std::string &key)
        {
            return query_params.find(key)->second;
        }

        std::vector<std::string> query_array_param(const std::string &key)
        {
            std::vector<std::string> out;
            auto range = query_params.equal_range(key);
            for (auto i = range.first; i != range.second; ++i)
            {
                out.push_back(i->second);
            }
            return out;
        }
    };
}
