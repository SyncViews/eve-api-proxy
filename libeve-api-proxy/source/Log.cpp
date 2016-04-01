#include "Precompiled.hpp"
#include "Log.hpp"

namespace
{
    const char *SEV_FATAL = "Fatal";
    const char *SEV_ERROR = "Error";
    const char *SEV_WARNING = "Warning";
    const char *SEV_INFO = "Info";
    const char *SEV_DEBUG = "Debug";
    thread_local std::stringstream log_buf;
    std::ostream &log_os = std::cout;

    tm localtime(time_t t)
    {
        tm tm;
#ifdef _WIN32
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        return tm;
    }
    tm localtime_now()
    {
        return localtime(time(nullptr));
    }
}


LogStream::LogStream(const char *sev)
    : sev(sev)
{
    if (sev)
    {
        log_buf.copyfmt(std::ios(NULL));
        //YYYY-MM-DD HH:MM:SS 
        char time_buf[(4 + 1 + 2 + 1 + 2) + 1 + (2 + 1 + 2 + 1 + 2) + 1] = { 0 };
        tm tm = localtime_now();
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &tm);

        log_buf << time_buf << ' ' << sev << ':' << ' ';
    }
}
LogStream::~LogStream()
{
    log_os << log_buf.str();
    log_os.flush();
    log_buf.str(std::string());
}
std::ostream &LogStream::get_os()
{
    return log_buf;
}


log_fatal::log_fatal()      : LogStream("Fatal") {}
log_error::log_error()      : LogStream("Error") {}
log_warning::log_warning()  : LogStream("Warning") {}
log_info::log_info()        : LogStream("Info") {}
log_debug::log_debug()      : LogStream(nullptr) {}
