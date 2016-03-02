#pragma once

/**For Windows*/
std::string utf16_to_utf8(const std::wstring &utf16);
std::string utf16_to_utf8(const std::u16string &utf16);
std::wstring utf8_to_utf16(const std::string &utf8);
