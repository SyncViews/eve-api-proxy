#include "Precompiled.hpp"
#include "Init.hpp"
#include "Server.hpp"
#include <iostream>

int main()
{
    init();
    
    Server server;
    log_info() << "Start" << std::endl;
    server.run();
    log_info() << "Waiting for terminate" << std::endl;

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
    log_info() << "Stopping" << std::endl;
    server.stop();
    log_info() << "Stopped" << std::endl;

    return 0;
}
