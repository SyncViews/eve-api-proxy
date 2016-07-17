#include "Precompiled.hpp"
#include "http/HttpStatusErrors.hpp"
#include "http/core/HttpRequest.hpp"
#include "model/EveRegions.hpp"

std::vector<int> params_regions(http::HttpRequest &request)
{
    std::vector<int> regions;
    if (request.url.query_param("region") == "all")
    {
        regions.assign(EVE_REGION_IDS, EVE_REGION_IDS + sizeof(EVE_REGION_IDS) / sizeof(EVE_REGION_IDS[0]));
    }
    else
    {
        for (auto &i : request.url.query_array_param("region"))
        {
            regions.push_back(std::stoi(i));
        }
        if (regions.empty()) throw http::HttpBadRequest("At least one region required.");
    }
    return regions;
}

std::vector<int> params_inv_types(http::HttpRequest &request)
{
    std::vector<int> types;
    for (auto &i : request.url.query_array_param("type"))
    {
        types.push_back(std::stoi(i));
    }
    if (types.empty()) throw http::HttpBadRequest("At least one type required.");
    return types;
}
