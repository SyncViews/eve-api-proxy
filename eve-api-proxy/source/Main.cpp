#include "Precompiled.hpp"
#include "Init.hpp"
#include "Server.hpp"
#include <iostream>

int main()
{
    init();
    
    Server server;
    std::cout << "Start" << std::endl;
    server.run();
    std::cout << "Waiting for terminate" << std::endl;

#ifdef _WIN32
    //TODO: Implement as a proper service
    std::cin.get();
#else
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    int sig;
    sigwait(&sigset, &sig);
#endif
    std::cout << "Stopping" << std::endl;
    server.stop();
    std::cout << "Stopped" << std::endl;

    return 0;
}
