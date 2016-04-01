#include "Precompiled.hpp"
#include "JsonReader.hpp"

JsonReader::JsonReader(const std::vector<uint8_t> &data)
{
    doc.Parse((const char*)data.data(), data.size());
    if (doc.HasParseError()) throw JsonReadError();
}

JsonReader::~JsonReader()
{
}

JsonReaderObject JsonReader::as_object()
{
    if (!doc.IsObject()) throw JsonReadError();
    return JsonReaderObject(&doc);
}

JsonReaderArray JsonReader::as_array()
{
    if (!doc.IsArray()) throw JsonReadError();
    return JsonReaderArray(&doc);
}

JsonReaderObject::JsonReaderObject(rapidjson::Value * value)
    : value(value)
{
}

rapidjson::Value *JsonReaderObject::get_native(const char *key)
{
    auto i = value->FindMember(key);
    if (i == value->MemberEnd()) throw JsonReadError();
    return &i->value;
}

JsonReaderArray JsonReaderObject::get_array(const char * key)
{
    auto v = get_native(key);
    if (!v->IsArray()) throw JsonReadError();
    return JsonReaderArray(v);
}

JsonReaderObject JsonReaderObject::get_object(const char * key)
{
    auto v = get_native(key);
    if (!v->IsObject()) throw JsonReadError();
    return JsonReaderObject(v);
}

int32_t JsonReaderObject::get_int32(const char * key)
{
    auto v = get_native(key);
    if (!v->IsInt()) throw JsonReadError();
    return v->GetInt();
}

int64_t JsonReaderObject::get_int64(const char * key)
{
    auto v = get_native(key);
    if (!v->IsInt64()) throw JsonReadError();
    return v->GetInt64();
}

float JsonReaderObject::get_float(const char * key)
{
    auto v = get_native(key);
    if (!v->IsFloat()) throw JsonReadError();
    return v->GetFloat();
}

double JsonReaderObject::get_double(const char * key)
{
    auto v = get_native(key);
    if (!v->IsDouble()) throw JsonReadError();
    return v->GetDouble();
}

std::string JsonReaderObject::get_str(const char * key)
{
    auto v = get_native(key);
    if (!v->IsString()) throw JsonReadError();
    return std::string(v->GetString(), v->GetStringLength());
}

JsonReaderArray::JsonReaderArray(rapidjson::Value * value)
    : value(value)
{
}

size_t JsonReaderArray::size()
{
    return value->Size();
}

JsonReaderObject JsonReaderArray::get_object(size_t i)
{
    return &value->GetArray()[(unsigned)i];
}
