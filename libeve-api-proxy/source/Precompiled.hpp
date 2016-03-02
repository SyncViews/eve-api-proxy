#pragma once
#define NOMINMAX //remove min and max macros, since they clash with things like std::numeric_limits
#define SECURITY_WIN32
#include <WinSock2.h> //Must go before windows.h
#include <Ws2tcpip.h>
#include <Windows.h>
#include <schannel.h>
#include <security.h>

#pragma comment(lib, "WSock32.Lib")
#pragma comment(lib, "Secur32.Lib")

#include <cassert>
#include <cstdint>
#include <cstring>

#include <algorithm>
#include <codecvt>
#include <exception>
#include <limits>
#include <locale>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <sstream>
