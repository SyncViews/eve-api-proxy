#pragma once
#include <cstdint>
#include <vector>
std::vector<uint8_t> gzip_decompress(const std::vector<uint8_t> &compressed);
