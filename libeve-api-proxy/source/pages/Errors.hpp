#pragma once
#include <http/Request.hpp>
#include <http/Response.hpp>

http::Response http_simple_error_page(http::Request &request, int code, const std::string &message);
http::Response http_bad_request(http::Request &request, const std::string &message);
