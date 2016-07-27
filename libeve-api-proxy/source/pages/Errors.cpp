#include "Precompiled.hpp"
#include "Errors.hpp"
#include <json/Writer.hpp>

http::Response http_simple_error_page(http::Request &request, int code, const std::string &message)
{
    json::Writer json_writer;
    json_writer.start_obj();
    json_writer.prop("error", message);
    json_writer.end_obj();

    http::Response resp;
    resp.status_code(code);
    resp.body = json_writer.str();
    return resp;
}

http::Response http_bad_request(http::Request &request, const std::string &message)
{
    return http_simple_error_page(request, 400, message);
}
