#pragma once

#ifdef _WIN32
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #define NOMINMAX //remove min and max macros, since they clash with things like std::numeric_limits
    #define SECURITY_WIN32
    #include <WinSock2.h> //Must go before windows.h
    #include <Ws2tcpip.h>
    #include <Windows.h>
    #include <schannel.h>
    #include <security.h>

    #pragma comment(lib, "WSock32.Lib")
    #pragma comment(lib, "Secur32.Lib")
//Seems G++ 4.8.x still does not support C++11...
//on the plus side, this is only currently used in windows
#   include <codecvt>

//Replace some annoying macros
#undef ERROR
static const int ERROR = 0;
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#include <openssl/opensslconf.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#ifndef OPENSSL_THREADS
#   error OPENSSL_THREADS required
#endif
//linux just uses "int" so going to use the windows types...
typedef int SOCKET;
static const auto INVALID_SOCKET = (SOCKET)~0;
static const auto SOCKET_ERROR = -1;
static const auto SD_BOTH = SHUT_RDWR;

//BSD sockets are file descriptors, but this is not true on Window's, so use the explicit forms
inline void closesocket(SOCKET sock)
{
    close(sock);
}
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
