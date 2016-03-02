#include "Precompiled.hpp"
#include "Init.hpp"

void init()
{
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
}
