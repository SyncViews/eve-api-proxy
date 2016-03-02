#include "Precompiled.hpp"
#include "BaseSocket.hpp"
#include "Error.hpp"

void BaseSocket::send_all(const uint8_t *bytes, size_t len)
{
    while (len)
    {
        auto len2 = send(bytes, len);
        if (!len2) throw NetworkError("Socket closed before send_all complete");
        len -= len2;
        bytes += len2;
    }
}
void BaseSocket::recv_all(uint8_t *bytes, size_t len)
{
    while (len)
    {
        auto len2 = recv(bytes, len);
        if (!len2) throw NetworkError("Socket closed before recv_all complete");
        len -= len2;
        bytes += len2;
    }
}
