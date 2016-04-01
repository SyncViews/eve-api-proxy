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

std::vector<std::string> splitString(const std::string &str, char delim)
{
    std::vector<std::string> ret;
    size_t p = 0, p2;
    while ((p2 = str.find(delim, p)) != std::string::npos)
    {
        ret.push_back(str.substr(p, p2 - p));
        p = p2 + 1;
    }
    ret.push_back(str.substr(p));
    return ret;
}
