#include "Precompiled.hpp"
#include "HttpResponseParser.hpp"

namespace http
{
    HttpResponseParser::HttpResponseParser()
    {
        reset();
    }
    void HttpResponseParser::reset()
    {
        parser_status = NOT_STARTED;
        status_code = 0;
        status_message.clear();
        headers.headers.clear();
        expected_body_len = 0;
        body.clear();
        buffer.clear();
    }
    size_t HttpResponseParser::read(const uint8_t * data, size_t len)
    {
        std::string line;
        auto p = data, end = data + len;

        while (p != end)
        {
            size_t len2 = 0;
            if (parser_status == NOT_STARTED && read_line(&line, &len2, p, end - p))
            {
                assert(buffer.empty());
                parser_status = READING_HEADERS;
                auto ver_end = line.find(' ', 0);
                auto code_end = line.find(' ', ver_end + 1);
                std::string code_str = line.substr(ver_end + 1, code_end - ver_end);
                status_code = std::stoi(code_str);
                status_message = line.substr(code_end + 1);
            }
            p += len2; len2 = 0;

            if (parser_status == READING_HEADERS && read_line(&line, &len2, p, end - p))
            {
                assert(buffer.empty());
                if (line.empty())
                {
                    //end of headers
                    if (headers.get("Transfer-Encoding") == "chunked")
                    {
                        expected_body_len = 0;
                        parser_status = READING_BODY_CHUNKED_LENGTH;
                    }
                    else
                    {
                        auto header_len = headers.find("Content-Length");
                        if (!header_len) throw std::runtime_error("'Content-Length' or 'Transfer-Encoding: chunked' required");
                        expected_body_len = std::stoul(header_len->value);
                        parser_status = READING_BODY;
                    }
                }
                else add_header(line);
            }
            p += len2; len2 = 0;

            if (parser_status == READING_BODY || parser_status == READING_BODY_CHUNKED)
            {
                assert(!buffer.size());
                assert(expected_body_len > 0);
                auto remaining = expected_body_len - body.size();
                size_t len = end - p;
                if (len >= remaining)
                {
                    body.insert(body.end(), p, p + remaining);
                    p += remaining;
                    parser_status = parser_status == READING_BODY ? COMPLETED : READING_BODY_CHUNKED_TERMINATOR;
                    expected_body_len = 0;
                }
                else
                {
                    body.insert(body.end(), p, p + len);
                    p = end;
                }
            }

            if (parser_status == READING_BODY_CHUNKED_TERMINATOR && read_line(&line, &len2, p, end - p))
            {
                if (line.empty()) parser_status = READING_BODY_CHUNKED_LENGTH;
                else throw std::runtime_error("Expected chunk \r\n terminator");
            }
            p += len2; len2 = 0;

            if (parser_status == READING_BODY_CHUNKED_LENGTH && read_line(&line, &len2, p, end - p))
            {
                expected_body_len = std::stoul(line, nullptr, 16);
                if (expected_body_len) parser_status = READING_BODY_CHUNKED;
                else parser_status = READING_TRAILER_HEADERS;
            }
            p += len2; len2 = 0;
            
            if (parser_status == READING_TRAILER_HEADERS && read_line(&line, &len2, p, end - p))
            {
                assert(buffer.empty());
                if (line.empty()) parser_status = COMPLETED; //end of headers
                else add_header(line);
            }
            p += len2; len2 = 0;
        }

        assert(p > data && p <= end);
        assert(p == end || is_completed());
        return p - data;
    }
    bool HttpResponseParser::read_line(std::string * line, size_t *consumed_len, const uint8_t *data, size_t len)
    {
        //The last byte of buffer might be a \r, and the first byte of data might be a \n
        if (buffer.size() && buffer.back() == '\r' && data[0] == '\n')
        {
            line->assign(buffer.data(), buffer.data() + buffer.size() - 1); //assign, but exclude the \r
            buffer.clear();
            *consumed_len =  1; //1 for the \n
            return true;
        }

        auto p = data;
        auto end = data + len - 1;
        while (true)
        {
            //looking for \r\n, so search for \r first and ignore the last byte
            auto p2 = (const uint8_t*)memchr(p, '\r', end - p);
            if (p2)
            {
                if (p2[1] == '\n')
                {
                    //found end of string
                    line->assign(buffer.data(), buffer.data() + buffer.size());
                    line->insert(line->end(), data, p2);
                    *consumed_len = p2 - data + 2;
                    buffer.clear();
                    return true;
                }
                else
                {
                    //just a \r by itself, look for the next one
                    p = p2 + 1;
                }
            }
            else break; //reached end of buffer
        }
        //not found, so all of data must be part of the line
        buffer.insert(buffer.end(), data, data + len);
        *consumed_len = len;
        return false;
    }

    void HttpResponseParser::add_header(const std::string &line)
    {
        auto colon = line.find(':', 0);
        auto first_val = line.find_first_not_of(' ', colon + 1);
        std::string name = line.substr(0, colon);
        std::string value = line.substr(first_val);
        headers.add(name, value);
    }
}
