#include "Precompiled.hpp"
#include "String.hpp"

#ifdef _WIN32
std::string utf16_to_utf8(const std::wstring &utf16)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    return convert.to_bytes(utf16);
}

//TODO: MSVC 2015 runtime seems to not include the char16_t version of this
/*std::string utf16_to_utf8(const std::u16string &utf16)
{
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    return convert.to_bytes(utf16);
}*/

std::wstring utf8_to_utf16(const std::string &utf8)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    return convert.from_bytes(utf8);
}
#endif

