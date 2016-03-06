#pragma once
#include "http/core/HttpRequest.hpp"
#include "http/core/HttpResponse.hpp"

http::HttpResponse http_simple_error_page(http::HttpRequest &request, int code, const std::string &message);
http::HttpResponse http_bad_request(http::HttpRequest &request, const std::string &message);
