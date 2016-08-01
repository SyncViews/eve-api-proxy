#pragma once

#ifdef _WIN32
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #define NOMINMAX //remove min and max macros, since they clash with things like std::numeric_limits
    #define SECURITY_WIN32
    #include <WinSock2.h> //Must go before windows.h
    #include <Windows.h>
//Seems G++ 4.8.x still does not support C++11...
//on the plus side, this is only currently used in windows
#   include <codecvt>

//Replace some annoying macros
#undef ERROR
static const int ERROR = 0;
#else
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#endif

#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <ctime>

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <locale>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <sstream>

#pragma comment(lib, "Secur32.Lib")
#pragma comment(lib, "zlibstat.lib")


#ifdef _WIN32
inline void set_thread_name(const std::string &str)
{
    const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)
    struct ThreadInfo
    {
        DWORD dwType; // Must be 0x1000.
        LPCSTR szName; // Pointer to name (in user addr space).
        DWORD dwThreadID; // Thread ID (-1=caller thread).
        DWORD dwFlags; // Reserved for future use, must be zero.
    };
#pragma pack(pop)
    DWORD threadId = GetCurrentThreadId();
    ThreadInfo info = { 0x1000, str.c_str(), threadId, 0 };
#pragma warning(push)
#pragma warning(disable: 6320 6322)
    __try {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
#pragma warning(pop)
}
#else
inline void set_thread_name(const std::string &str)
{
    pthread_setname_np(pthread_self(), str.c_str());
}
#endif

#include "Log.hpp"
