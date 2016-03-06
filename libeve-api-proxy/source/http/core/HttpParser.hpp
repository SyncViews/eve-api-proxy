#pragma once
#include "HttpHeaders.hpp"
#include "../../Precompiled.hpp"
#include <cstdint>
#include <string>
#include <vector>
namespace http
{
    class HttpParser
    {
    public:
        enum ParserStatus
        {
            NOT_STARTED,
            READING_HEADERS,
            READING_BODY,
            READING_BODY_CHUNKED,
            READING_BODY_CHUNKED_LENGTH,
            READING_BODY_CHUNKED_TERMINATOR,
            READING_TRAILER_HEADERS,
            COMPLETED
        };

        explicit HttpParser(bool request_parser);
        void reset();

        size_t read(const uint8_t *data, size_t len);
        int get_status_code()const { return status_code; }
        const std::string &get_status_message()const { return status_message; }

        const std::string &get_method()const { return method; }
        const std::string &get_url()const { return url; }

        bool is_completed()const { return parser_status == COMPLETED; }
        const std::vector<uint8_t> &get_body()const { return body; }
        const HttpHeaders &get_headers()const { return headers; }
    private:
        bool request_parser;
        ParserStatus parser_status;
        int status_code;
        std::string status_message;
        std::string method;
        std::string url;
        HttpHeaders headers;
        /**The expected final length of body from the Content-Length header.*/
        size_t expected_body_len;
        /**The body data*/
        std::vector<uint8_t> body;
        /**Holds data that is definately part of this response (e.g. part of a header)
         * while waiting for 'read' to be called again with more data.
         */
        std::vector<uint8_t> buffer;

        /**Try to read a line from this->buffer + data.
         * 
         * @param line If a line is read, this is assigned its value not including the \r\n.
         * @param consumed_len The number of bytes of data consumed. They may have been added to
         * either this->buffer or line.
         * @param data The input data.
         * @param len The length of the input data.
         * @return True if a line was read. Data may be consumed and saved in this->buffer without
         * a full line being read, with false being returned but consumed_len greater than 0.
         */
        bool read_line(std::string *line, size_t *consumed_len, const uint8_t *data, size_t len);

        void add_header(const std::string &line);
    };
}
