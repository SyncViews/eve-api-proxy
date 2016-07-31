#pragma once

std::string errno_string(int err);

#ifdef _WIN32

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
#endif
