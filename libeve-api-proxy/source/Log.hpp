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
    void write(const char *str, std::streamsize len)
    {
        if (sev) get_os().write(str, len);
    }
private:
    const char *sev;
};


struct log_fatal   : public LogStream { log_fatal(); };
struct log_error   : public LogStream { log_error(); };
struct log_warning : public LogStream { log_warning(); };
struct log_info    : public LogStream { log_info(); };
struct log_debug   : public LogStream { log_debug(); };
