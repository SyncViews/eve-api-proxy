#pragma once
#include "../Precompiled.hpp"
#include <cstdint>
#include <string>
class BaseSocket
{
public:
    virtual ~BaseSocket() {}
    /**Get a printable address / info string for this socket.*/
    virtual std::string address_str()const = 0;
    virtual size_t send(const uint8_t *bytes, size_t len) = 0;
    virtual size_t recv(uint8_t *bytes, size_t len) = 0;

    void send_all(const uint8_t *bytes, size_t len);
    void recv_all(uint8_t *bytes, size_t len);
};
