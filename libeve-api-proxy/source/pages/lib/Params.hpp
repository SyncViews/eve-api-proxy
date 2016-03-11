#pragma once
#include "http/core/HttpRequest.hpp"
#include <vector>

std::vector<int> params_regions(http::HttpRequest &request);
std::vector<int> params_inv_types(http::HttpRequest &request);
