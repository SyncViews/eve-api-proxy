#include "Precompiled.hpp"
#include "Gzip.hpp"
#define ZLIB_CONST
#include <zlib.h>

std::vector<uint8_t> gzip_decompress(const std::vector<uint8_t> &compressed)
{
    static const size_t BUFFER_LEN = 0x40000;
    std::vector<uint8_t> out;
    std::unique_ptr<uint8_t> buffer(new uint8_t[BUFFER_LEN]);
    z_stream stream = {0};//TODO: Leak on exception
    auto ret = inflateInit2(&stream, MAX_WBITS | 16);
    if (ret != Z_OK) throw std::runtime_error("zlib inflateInit failed");

    stream.next_in = compressed.data();
    stream.avail_in = (unsigned)compressed.size();

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
        auto ready_out = BUFFER_LEN - stream.avail_out;
        out.insert(out.end(), buffer.get(), buffer.get() + ready_out);
    }
    while (ret != Z_STREAM_END);

    inflateEnd(&stream);
    return out;
}

