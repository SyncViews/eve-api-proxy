#include "Precompiled.hpp"
#include "Init.hpp"

SecurityFunctionTableW *sspi;

void init()
{
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);

    sspi = InitSecurityInterfaceW();
}
