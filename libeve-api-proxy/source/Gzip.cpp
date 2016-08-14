#include "Precompiled.hpp"
#include "Gzip.hpp"
#define ZLIB_CONST
#include <zlib.h>

namespace
{
    class OutBuffer
    {
    public:
        OutBuffer(unsigned capacity)
            : _len(0), _capacity(capacity)
            , _ptr(new char[capacity])
        {}

        Bytef *data() { return (Bytef*)_ptr.get(); }
        unsigned capacity()const { return _capacity; }
        unsigned size()const { return _len; }
        void added(unsigned len)
        {
            _len += len;
            assert(_len <= _capacity);
        }
        __declspec(noinline) void enlarge()
        {
            auto new_cap = _capacity * 2;
            std::unique_ptr<char[]> new_ptr(new char[new_cap]);
            memcpy(new_ptr.get(), _ptr.get(), _len);
            _ptr = std::move(new_ptr);
            _capacity = new_cap;
        }

        std::unique_ptr<char[]>&& get() { return std::move(_ptr); }
    private:
        unsigned _len, _capacity;
        std::unique_ptr<char[]> _ptr;
    };

    class ZStream
    {
    public:
        ZStream()
        {
            memset(&stream, 0, sizeof(stream));
            auto ret = inflateInit2(&stream, MAX_WBITS | 16);
            if (ret != Z_OK) throw std::runtime_error("zlib inflateInit failed");
        }
        ~ZStream()
        {
            inflateEnd(&stream);
        }
        z_stream *get() { return &stream; }
        z_stream *operator -> () { return &stream; }
    private:
        z_stream stream;
    };
}
GzipOut gzip_decompress(const uint8_t *start, size_t len)
{
    //dont even try if input is more than 8MB and initial out buffer is more than 160MB, the
    //memory contraints on the server are not intended for anywhere near so much
    if (len > 1024*1024*8) throw std::runtime_error("gzip data too large");
    OutBuffer out(std::max<unsigned>(4096, (unsigned)len * 20));
    ZStream stream;
    stream->next_in = start;
    stream->avail_in = (uInt)len;

    while (true)
    {
        //out buffer
        stream->next_out = out.data() + out.size();
        unsigned avail_out = out.capacity() - out.size() - 1;
        stream->avail_out = avail_out;
        //decompress
        auto ret = inflate(stream.get(), Z_NO_FLUSH);
        out.added(avail_out - stream->avail_out);
        if (ret == Z_STREAM_END)
        {
            break;
        }
        else if (ret == Z_OK)
        {
            if (!stream->avail_in) throw std::runtime_error("zlib not finished but no more input");
            out.enlarge();
        }
        else throw std::runtime_error("zlib error");
    }
    return {out.get(), out.size()};
}

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
        out.append((const char*)bytes, len);
    };
    gzip_decompress((const uint8_t*)compressed.data(), compressed.size(), f);
    return out;
}
