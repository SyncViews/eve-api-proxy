#pragma once

std::string win_error_string(int err);
inline std::string wsa_error_string(int err)
{
    return win_error_string(err);
}

class NetworkError : public std::runtime_error
{
public:
    explicit NetworkError(const char *str) : std::runtime_error(str) {}
    explicit NetworkError(const std::string &str) : std::runtime_error(str) {}
};

class WsaError : public NetworkError
{
public:
    explicit WsaError(int err)
        : NetworkError(wsa_error_string(err))
    {}
    explicit WsaError()
        : WsaError(WSAGetLastError())
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
