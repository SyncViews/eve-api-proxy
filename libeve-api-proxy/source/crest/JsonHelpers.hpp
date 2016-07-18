#pragma once
#include <json/Reader.hpp>
#include <vector>
#include <cstdint>
namespace crest
{
    template<typename F>
    void read_crest_items(json::Parser &parser, F read_value)
    {
        bool found = false;
        auto tok = parser.next();
        if (tok.type != json::Token::OBJ_START) throw std::runtime_error("Expected object");
        do
        {
            tok = parser.next();
            if (tok.type != json::Token::STRING) throw std::runtime_error("Expected string");
            bool items = tok.str == "items";
            tok = parser.next();
            if (tok.type != json::Token::KEY_SEP) throw std::runtime_error("Expected ':' after key");
            if (items)
            {
                read_value();
                found = true;
            }
            else json::skip_value(parser);
            tok = parser.next();
        } while (tok.type == json::Token::ELEMENT_SEP);
        if (tok.type != json::Token::OBJ_END) throw std::runtime_error("Expected object end");
        if (!found) throw std::runtime_error("CREST 'items' not found");
    }

    template<typename T>
    void read_crest_items(json::Parser &parser, T *out)
    {
        read_crest_items(parser, [&parser, out]() { read_json(parser, out); });
    }

    template<typename T> T read_crest_items(const std::vector<uint8_t> &data)
    {
        json::Parser parser(
            (const char*)data.data(),
            (const char*)data.data() + data.size());
        T tmp;
        read_crest_items(parser, &tmp);
        return tmp;
    }
}
