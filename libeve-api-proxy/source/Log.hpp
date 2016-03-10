#pragma once
#include <iostream>
#include <string>
#include <ctime>

class LogStream
{
public:
    LogStream(const char *sev);
    ~LogStream();

    std::ostream &get_os();


    template<class T>
    LogStream& operator << (const T &arg)
    {
        if (sev) { get_os() << arg; }
        return *this;
    }
    LogStream& operator << (std::ostream & (*manipulator)(std::ostream &))
    {
        if (sev) manipulator(get_os());
        return *this;
    }
private:
    const char *sev;
};


struct log_fatal   : public LogStream { log_fatal()   : LogStream("Fatal")   {} };
struct log_error   : public LogStream { log_error()   : LogStream("Error")   {} };
struct log_warning : public LogStream { log_warning() : LogStream("Warning") {} };
struct log_info    : public LogStream { log_info()    : LogStream("Info")    {} };
struct log_debug   : public LogStream { log_debug()   : LogStream(nullptr)   {} };
