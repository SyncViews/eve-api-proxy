#pragma once
#define NOMINMAX //remove min and max macros, since they clash with things like std::numeric_limits
#include <WinSock2.h> //Must go before windows.h
#include <Ws2tcpip.h>
#include <windows.h>

#include <cassert>
#include <cstdint>

#include <codecvt> 
#include <exception>
#include <limits>
#include <locale>
#include <memory>
#include <stdexcept>
#include <string>

#include <sstream>
