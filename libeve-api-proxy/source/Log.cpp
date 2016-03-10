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
}


LogStream::LogStream(const char *sev)
    : sev(sev)
{
    if (sev)
    {
        log_buf.copyfmt(std::ios(NULL));
        //YYYY-MM-DD HH:MM:SS 
        char time_buf[(4 + 1 + 2 + 1 + 2) + 1 + (2 + 1 + 2 + 1 + 2) + 1] = { 0 };
        auto now = time(nullptr);
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&now));

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

