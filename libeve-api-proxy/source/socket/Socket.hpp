#pragma once
#include "../Precompiled.hpp"
#include <cstdint>
#include <string>
class Socket
{
public:
    virtual ~Socket() {}

    virtual void connect(const std::string &host, uint16_t port) = 0;
    virtual bool is_connected()const = 0;

    virtual void close() = 0;

    /**Get a printable address / info string for this socket.*/
    virtual std::string address_str()const = 0;
    virtual size_t send(const uint8_t *bytes, size_t len) = 0;
    virtual size_t recv(uint8_t *bytes, size_t len) = 0;

    void send_all(const uint8_t *bytes, size_t len);
    void recv_all(uint8_t *bytes, size_t len);
};
