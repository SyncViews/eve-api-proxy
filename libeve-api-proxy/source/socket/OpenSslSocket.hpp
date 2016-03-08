#pragma once
#include "Socket.hpp"
#include "TcpSocket.hpp"

class TlsSocket : public Socket
{
public:
    TlsSocket();
    ~TlsSocket();

    virtual void connect(const std::string & host, uint16_t port) override;

    virtual bool is_connected() const override;

    virtual void close() override;

    virtual std::string address_str() const override;

    virtual size_t send(const uint8_t * bytes, size_t len) override;

    virtual size_t recv(uint8_t * bytes, size_t len) override;
private:
    TcpSocket tcp;
    SSL *ssl;
};
