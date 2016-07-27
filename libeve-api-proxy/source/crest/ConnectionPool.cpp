#include "Precompiled.hpp"
#include "ConnectionPool.hpp"
#include "Error.hpp"
#include "Cache.hpp"
#include "CppServers.hpp"
#include "http/core/HttpParser.hpp"
#include "Gzip.hpp"
#include <iostream>
namespace crest
{
    namespace
    {
        http::AsyncClientParams client_params(http::SocketFactory *socket_factory)
        {
            http::AsyncClientParams ret;
            ret.tls = true;
            ret.host = PCREST_HOST;
            ret.port = PCREST_PORT;
            ret.max_connections = PCREST_MAX_CONNECTIONS;
            ret.rate_limit = PCREST_MAX_REQS_PER_SEC;
            ret.socket_factory = socket_factory;
            ret.default_headers.add("Accept-Encoding", "gzip");
            ret.default_headers.add("User-Agent", "eve-api-proxy");

            return ret;
        };
    }
    ConnectionPool::ConnectionPool()
        : socket_factory()
        , http_client(client_params(&socket_factory))
    {
    }

    ConnectionPool::~ConnectionPool()
    {
        exit();
    }

    void ConnectionPool::exit()
    {
        //TODO: Not implemented
    }

    void ConnectionPool::queue(http::AsyncRequest *request)
    {
        http_client.make_request(request);
    }
}
