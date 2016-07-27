#pragma once
#include <http/Request.hpp>
#include <vector>

std::vector<int> params_regions(http::Request &request);
std::vector<int> params_inv_types(http::Request &request);
