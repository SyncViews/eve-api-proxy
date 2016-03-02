#include "Precompiled.hpp"
#include "TcpSocket.hpp"
#include "Error.hpp"
std::string TcpSocket::address_str(const std::string &host, uint16_t port)
{
    std::stringstream ss;
    ss << host << ":" << port;
    return ss.str();
}

TcpSocket::TcpSocket()
    : host(), port(0), sock()
{}
TcpSocket::~TcpSocket()
{
    if (is_connected()) close();
    assert(!is_connected());
}

void TcpSocket::connect(const std::string &host, uint16_t port)
{
    assert(!is_connected());

    auto port_str = std::to_string(port);
    addrinfo *result = nullptr;
    
    addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    auto ret = getaddrinfo(host.c_str(), port_str.c_str(), &hints, &result);
    if (ret) throw WsaError();

    int lastError = 0;
    for (auto p = result; p; p = p->ai_next)
    {
        SOCKET sock =:: socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock == INVALID_SOCKET) continue;

        if (::connect(sock, p->ai_addr, (int)p->ai_addrlen) == SOCKET_ERROR)
        {
            lastError = WSAGetLastError();
            ::closesocket(sock);
            continue;
        }
        //TODO: Dont leak memory on error
        freeaddrinfo(result);

        this->sock = sock;
        this->host = host;
        this->port = port;
        assert(is_connected());
        return;
    }
    //TODO: Better error report
    throw NetworkError("Failed to connect to " + address_str(host, port));
}
bool TcpSocket::is_connected()const
{
    return !host.empty();
}

void TcpSocket::close()
{
    if (is_connected())
    {
        if(shutdown(sock, SD_BOTH))
            throw WsaError();
        ::closesocket(sock);
        sock = 0;
        host.clear();
        port = 0;
    }
    assert(!is_connected());
}

std::string TcpSocket::address_str()const
{
    if (is_connected()) return address_str(host, port);
    else return "Not connected";
}

size_t TcpSocket::send(const uint8_t *bytes, size_t len)
{
    assert(is_connected());
    assert(len < std::numeric_limits<int>::max());
    auto ret = ::send(sock, (const char*)bytes, (int)len, 0);
    if (ret == SOCKET_ERROR) throw WsaError();
    return (size_t) ret;
}

size_t TcpSocket::recv(uint8_t *bytes, size_t len)
{
    assert(is_connected());
    assert(len < std::numeric_limits<int>::max());
    auto ret = ::recv(sock, (char*)bytes, (int)len, 0);
    if (ret == SOCKET_ERROR) throw WsaError();
    return (size_t)ret;
}
