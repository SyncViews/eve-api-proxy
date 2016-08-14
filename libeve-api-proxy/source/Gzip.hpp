#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

/**Data buffer for gzip_decompress output.
 * Garuntees "data[size]" is a valid element, to allow for the creation of a
 * null-terminated string without a new memory allocation.
 */
struct GzipOut
{
    std::unique_ptr<char[]> data;
    unsigned size;
};
GzipOut gzip_decompress(const uint8_t *start, size_t len);
std::vector<uint8_t> gzip_decompress(const std::vector<uint8_t> &compressed);
std::string gzip_decompress(const std::string &compressed);
