#include "Precompiled.hpp"
#include "Server.hpp"
#include "Error.hpp"
#include "http/core/HttpParser.hpp"
#include "http/core/HttpRequest.hpp"
#include "http/core/HttpResponse.hpp"
#include "http/core/HttpStatusCode.hpp"
#include "http/core/HttpWriter.hpp"

#include "pages/Errors.hpp"
#include "pages/BulkMarketOrders.hpp"

#include <iostream>

Server::Server()
    : exiting(false), connections(), listen_socket(INVALID_SOCKET)
{
}

Server::~Server()
{
    closesocket(listen_socket);
    exiting = true;
    connections.clear();
}

void Server::run()
{
    exiting = false;
    thread = std::thread(&Server::server_main, this);
}

void Server::stop()
{
    exiting = true;
    ::shutdown(listen_socket, SD_BOTH);
    ::closesocket(listen_socket);
    listen_socket = INVALID_SOCKET;
    thread.join();

    for (auto & conn : connections)
    {
        //TODO: Might not be safe!
        conn.socket.close();
        conn.thread.join();
    }
    connections.clear();

    crest_cache.stop();
}

void Server::server_main()
{
    //create socket
    listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket == INVALID_SOCKET) throw std::runtime_error("Failed to create listen socket");
    //bind socket
    sockaddr_in bind_addr = { 0 };
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(5000);
    bind_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    if (bind(listen_socket, (const sockaddr*)&bind_addr, sizeof(bind_addr)))
        throw std::runtime_error("Failed to bind listen socket");
    //listen
    if (listen(listen_socket, 10)) throw std::runtime_error("Socket listen failed");
    
    //Server loop
    while (!exiting)
    {
        //accept
        sockaddr_in client_addr = { 0 };
        int client_addr_len = (int)sizeof(client_addr);
        auto client_socket = accept(listen_socket, (sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == INVALID_SOCKET)
        {
            auto err = WSAGetLastError();
            if (err == WSAEINTR) break; //exit
            else throw WsaError(err);
        }
        //process
        connections.emplace_back(this, client_socket, client_addr);
        //also clear out any dead entries
        for (auto conn = connections.begin(); conn != connections.end(); )
        {
            if (conn->thread_running) ++conn;
            else
            {
                conn->thread.join();
                conn = connections.erase(conn);
            }
        }
        std::cout << connections.size() << " active server connections" << std::endl;
    }
}

Server::ServerConnection::ServerConnection(Server *server, SOCKET socket, sockaddr_in addr)
    : server(server), thread_running(true), thread(), socket(socket, addr)
{
    thread = std::thread(&Server::ServerConnection::main, this);
}

Server::ServerConnection::~ServerConnection()
{
}

void Server::ServerConnection::main()
{
    try
    {
        main2();
    }
    catch (const std::exception &e)
    {
        thread_running = false;
        std::cerr << "HTTP server connection thread crashed. Terminating.\n" << e.what() << std::endl;
    }
}

void Server::ServerConnection::main2()
{
    http::HttpParser parser(true);
    uint8_t buffer[4096];
    size_t buffer_len = 0;
    while (!server->exiting && socket.is_connected())
    {
        size_t buffer_extra_p, buffer_extra_len = 0;
        parser.reset();
        while (!parser.is_completed())
        {
            //read from socket
            auto len = socket.recv(buffer + buffer_len, sizeof(buffer) - buffer_len) + buffer_len;
            if (len == 0)
            {   // disconnect
                return;
            }
            buffer_len = 0;
            //parse
            auto len2 = parser.read(buffer, len);
            assert(parser.is_completed() || len == len2);
            if (parser.is_completed())
            {
                //extra data, maybe pipelined request
                buffer_extra_p = len;
                buffer_extra_len = len2 - len;

                //Connection: header status
                bool keep_alive = !server->exiting && parser.get_headers().get("Connection") == "keep-alive";

                //process request
                http::HttpRequest request = {
                    parser.get_method(),
                    http::UrlParser(parser.get_url()),
                    parser.get_headers() };
                auto response = process_request(request);

                //Default headers
                response.headers.set_default("Content-Type", "application/json; charset=UTF-8");
                //response.headers.set_default("Content-Encoding", "gzip");
                response.headers.set_default("Content-Length", std::to_string(response.body.size()));
                response.headers.set_default("Connection", keep_alive ? "keep-alive" : "close");

                //response header
                std::stringstream response_ss;
                response_ss << "HTTP1/1 " << response.status_code << " " << http::get_status_code_str(response.status_code) << "\r\n";
                http::write_http_headers(response_ss, response.headers);
                response_ss << "\r\n";
                std::string response_head = response_ss.str();
                socket.send_all((const uint8_t*)response_head.data(), response_head.size());

                //response body
                if (!response.body.empty())
                {
                    socket.send_all(response.body.data(), response.body.size());
                }

                if (!keep_alive)
                {
                    socket.close();
                    return;
                }
            }
        }
        if (buffer_extra_len)
        {
            memmove(buffer, buffer + buffer_extra_p, buffer_extra_len);
        }
    }
}

http::HttpResponse Server::ServerConnection::process_request(http::HttpRequest &request)
{
    if (request.url.path == "/bulk-market-orders")
    {
        return http_get_bulk_market_orders(server->crest_cache, request);
    }
    else return http_simple_error_page(request, 404, request.url.path + " was not found");
}
