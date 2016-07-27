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
    //TODO: implement
}
#else
inline void set_thread_name(const std::string &str)
{
    //TODO: implement
    pthread_setname_np(pthread_self(), str.c_str());
}
#endif

#include "Log.hpp"
