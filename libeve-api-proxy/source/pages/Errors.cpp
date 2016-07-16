#include "Precompiled.hpp"
#include "Errors.hpp"
#include <json/Writer.hpp>

http::HttpResponse http_simple_error_page(http::HttpRequest &request, int code, const std::string &message)
{
    json::Writer json_writer;
    json_writer.start_obj();
    json_writer.prop("error", message);
    json_writer.end_obj();

    http::HttpResponse resp;
    resp.status_code = code;
    resp.body.assign(
        (const uint8_t*)json_writer.str().data(),
        (const uint8_t*)json_writer.str().data() + json_writer.str().size());
    return resp;
}

http::HttpResponse http_bad_request(http::HttpRequest &request, const std::string &message)
{
    return http_simple_error_page(request, 400, message);
}
