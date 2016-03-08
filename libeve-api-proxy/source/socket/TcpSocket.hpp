#pragma once
#include "Socket.hpp"
class TcpSocket : public Socket
{
public:
    static std::string address_str(const std::string &host, uint16_t port);

    TcpSocket();
    TcpSocket(SOCKET sock, const sockaddr_in &addr);
    ~TcpSocket();

    const std::string &get_host()const { return host; }

    virtual void connect(const std::string &host, uint16_t port) override;
    virtual bool is_connected()const override;

    void shutdown();
    virtual void close() override;

    virtual std::string address_str()const override;
    virtual size_t send(const uint8_t *bytes, size_t len) override;
    virtual size_t recv(uint8_t *bytes, size_t len) override;


    SOCKET get_socket() { return sock; }
private:
    std::string host;
    uint16_t port;
    SOCKET sock;
};
