#pragma once
#include "crest/Cache.hpp"
#include <http/server/CoreServer.hpp>
#include <thread>

class Server : public http::CoreServer
{
public:
    Server();
    ~Server();

    void run();
    virtual http::Response handle_request(http::Request &request)override;
    virtual http::Response parser_error_page(const http::ParserError &err)override;
private:
    crest::Cache crest_cache;

};
