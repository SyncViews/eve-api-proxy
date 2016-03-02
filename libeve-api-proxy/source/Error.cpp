#include "Precompiled.hpp"
#include "Error.hpp"
#include "SmartPtr.hpp"
#include "String.hpp"

std::string wsa_error_string(int err)
{
    std::unique_ptr<wchar_t[], LocalFreeDeleter> buffer;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)(wchar_t**)unique_out_ptr(buffer), 0, nullptr);
    return utf16_to_utf8(buffer.get());
}