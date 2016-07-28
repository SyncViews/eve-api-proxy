#pragma once
#include <cstdint>
#include <string>
#include <vector>
std::vector<uint8_t> gzip_decompress(const std::vector<uint8_t> &compressed);
std::string gzip_decompress(const std::string &compressed);
