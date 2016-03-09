#include "Precompiled.hpp"
#include "Socket.hpp"
#include "Error.hpp"

void Socket::send_all(const uint8_t *bytes, size_t len)
{
    auto remaining = len;
    while (remaining)
    {
        auto len2 = send(bytes, remaining);
        if (!len2) throw NetworkError("Socket closed before send_all complete");
        remaining -= len2;
        bytes += len2;
    }
}
void Socket::recv_all(uint8_t *bytes, size_t len)
{
    auto remaining = len;
    while (remaining)
    {
        auto len2 = recv(bytes, remaining);
        if (!len2) throw NetworkError("Socket closed before recv_all complete");
        remaining -= len2;
        bytes += len2;
    }
}
