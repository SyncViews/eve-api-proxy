#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <rapidjson/document.h>

class JsonReadError : std::runtime_error
{
public:
    JsonReadError() : std::runtime_error("Error reading JSON") {}
};

class JsonReaderObject;
class JsonReaderArray;


class JsonReader
{
public:
    explicit JsonReader(const std::vector<uint8_t> &data);
    ~JsonReader();

    JsonReaderObject as_object();
    JsonReaderArray as_array();

    rapidjson::Document& native() { return doc; }
private:
    rapidjson::Document doc;
};

class JsonReaderObject
{
public:
    JsonReaderObject(rapidjson::Value *value);
    rapidjson::Value *get_native() { return value; }


    rapidjson::Value *get_native(const char *key);
    JsonReaderArray get_array(const char *key);
    JsonReaderObject get_object(const char *key);

    int32_t get_int32(const char *key);
    int64_t get_int64(const char *key);

    float get_float(const char *key);
    double get_double(const char *key);

    std::string get_str(const char *key);
private:
    rapidjson::Value *value;
};
class JsonReaderArray
{
public:
    JsonReaderArray(rapidjson::Value *value);

    rapidjson::Value *get_native() { return value; }

    size_t size();
    JsonReaderObject get_object(size_t i);
private:
    rapidjson::Value *value;
};
