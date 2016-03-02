#pragma once

std::string wsa_error_string(int err);

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
