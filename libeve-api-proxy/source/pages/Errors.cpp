#include "Precompiled.hpp"
#include "Errors.hpp"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

http::HttpResponse http_simple_error_page(http::HttpRequest &request, int code, const std::string &message)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    writer.StartObject();
    writer.Key("error"); writer.String(message.data(), (unsigned)message.size());
    writer.EndObject();

    http::HttpResponse resp;
    resp.status_code = code;
    resp.body.assign(
        (const uint8_t*)buffer.GetString(),
        (const uint8_t*)buffer.GetString() + buffer.GetSize());
    return resp;
}

http::HttpResponse http_bad_request(http::HttpRequest &request, const std::string &message)
{
    return http_simple_error_page(request, 400, message);
}
