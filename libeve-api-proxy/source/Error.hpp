#pragma once

std::string errno_string(int err);

class NetworkError : public std::runtime_error
{
public:
    explicit NetworkError(const char *str) : std::runtime_error(str) {}
    explicit NetworkError(const std::string &str) : std::runtime_error(str) {}
};

#ifdef _WIN32
std::string win_error_string(int err);
inline std::string wsa_error_string(int err)
{
    return win_error_string(err);
}


class SocketError : public NetworkError
{
public:
    explicit SocketError(int err)
        : NetworkError(wsa_error_string(err))
    {}
    explicit SocketError()
        : SocketError(WSAGetLastError())
    {}
};

class WinError : public std::runtime_error
{
public:
    explicit WinError(int err)
        : std::runtime_error(win_error_string(err))
    {}
    explicit WinError()
        : WinError(GetLastError())
    {}
};

typedef SocketError WsaError;

#else

class SocketError : public NetworkError
{
public:
    explicit SocketError(int err)
        : NetworkError(errno_string(err))
    {}
    explicit SocketError()
        : SocketError(errno)
    {}
    explicit SocketError(const char *str) : NetworkError(str) {}
    explicit SocketError(const std::string &str) : NetworkError(str) {}
};

class OpenSslSocketError : public SocketError
{
public:
    OpenSslSocketError(SSL *ssl, int error)
        : SocketError(ERR_reason_error_string(SSL_get_error(ssl, error)))
    {
    }
};


#endif
