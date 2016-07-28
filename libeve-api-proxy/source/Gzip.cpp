#include "Precompiled.hpp"
#include "Gzip.hpp"
#define ZLIB_CONST
#include <zlib.h>

template<typename OUT_F>
void gzip_decompress(const uint8_t *start, size_t len, OUT_F out_f)
{
    static const size_t BUFFER_LEN = 0x40000;
    std::unique_ptr<uint8_t> buffer(new uint8_t[BUFFER_LEN]);
    z_stream stream = { 0 };//TODO: Leak on exception
    auto ret = inflateInit2(&stream, MAX_WBITS | 16);
    if (ret != Z_OK) throw std::runtime_error("zlib inflateInit failed");

    stream.next_in = start;
    stream.avail_in = (uInt)len;

    do
    {
        if (!stream.avail_in) throw std::runtime_error("zlib not finished but no more input");
        //out buffer
        stream.next_out = buffer.get();
        stream.avail_out = (unsigned)BUFFER_LEN;
        //decompress
        ret = inflate(&stream, Z_NO_FLUSH);
        //errors
        if (ret != Z_OK && ret != Z_STREAM_END) throw std::runtime_error("zlib error");
        //write
        size_t ready_out = BUFFER_LEN - stream.avail_out;
        out_f(buffer.get(), ready_out);
    } while (ret != Z_STREAM_END);

    inflateEnd(&stream);
}

std::vector<uint8_t> gzip_decompress(const std::vector<uint8_t> &compressed)
{
    std::vector<uint8_t> out;
    auto f = [&out](const uint8_t *bytes, size_t len)
    {
        out.insert(out.end(), bytes, bytes + len);
    };
    gzip_decompress(compressed.data(), compressed.size(), f);
    return out;
}
std::string gzip_decompress(const std::string &compressed)
{
    std::string out;
    auto f = [&out](const uint8_t *bytes, size_t len)
    {
        out.insert(out.end(), bytes, bytes + len);
    };
    gzip_decompress((const uint8_t*)compressed.data(), compressed.size(), f);
    return out;
}
