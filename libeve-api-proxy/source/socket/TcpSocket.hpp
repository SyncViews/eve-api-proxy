#pragma once
#include "Socket.hpp"
class TcpSocket : public Socket
{
public:
    TcpSocket();
    ~TcpSocket();

    virtual void connect(const std::string &host, uint16_t port);
    virtual bool is_connected()const;

    virtual void close();

    virtual std::string address_str()const;
    virtual size_t send(const unsigned char *bytes, size_t len);
    virtual size_t recv(unsigned char *bytes, size_t len);
private:
    std::string host;
    uint16_t port;
    SOCKET sock;
};
