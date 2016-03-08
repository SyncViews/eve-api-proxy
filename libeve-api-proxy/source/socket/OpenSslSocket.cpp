#include "Precompiled.hpp"
#include "OpenSslSocket.hpp"
#include "Init.hpp"
#include "Error.hpp"

TlsSocket::TlsSocket()
    : tcp(), ssl(nullptr)
{
}

TlsSocket::~TlsSocket()
{
    if (is_connected()) close();
}

void TlsSocket::connect(const std::string & host, uint16_t port)
{
    tcp.connect(host, port);
    
    ssl = SSL_new(openssl_ctx);
    SSL_set_fd(ssl, tcp.get_socket());
    auto err = SSL_connect(ssl);
    if (err < 0) throw OpenSslSocketError(ssl, err);
}

bool TlsSocket::is_connected() const
{
    return tcp.is_connected();
}

void TlsSocket::close()
{
    if (ssl)
    {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        ssl = nullptr;
    }
    tcp.close();
}

std::string TlsSocket::address_str() const
{
    return tcp.address_str();
}

size_t TlsSocket::send(const uint8_t * bytes, size_t len)
{
    auto len2 = SSL_write(ssl, (const char*)bytes, len);
    if (len2 < 0) throw OpenSslSocketError(ssl, len2);
    return len2;
}

size_t TlsSocket::recv(uint8_t * bytes, size_t len)
{
    auto len2 = SSL_read(ssl, (char*)bytes, len);
    if (len2 < 0) throw OpenSslSocketError(ssl, len2);
    return len2;
}
