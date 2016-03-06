#pragma once
#include "socket/TcpSocket.hpp"
#include <thread>
namespace http
{
    class HttpRequest;
    class HttpResponse;
}
class Server
{
public:
    Server();
    ~Server();

    void run();
    void stop();
private:
    class ServerConnection
    {
    public:
        ServerConnection(Server *server, SOCKET socket, sockaddr_in addr);
        ~ServerConnection();

        void main();
    private:
        friend class Server;

        Server *server;
        std::thread thread;
        TcpSocket socket;

        http::HttpResponse process_request(http::HttpRequest &request);
    };

    std::atomic<bool> exiting;
    std::list<ServerConnection> connections;
    SOCKET listen_socket;
    std::thread thread;

    void server_main();
};
