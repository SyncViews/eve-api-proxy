#include "Precompiled.hpp"
#include "Init.hpp"
#include "Server.hpp"
#include <iostream>

int main()
{
    init();
    
    Server server;
    server.run();

    std::cin.get();

    server.stop();

    return 0;
}
