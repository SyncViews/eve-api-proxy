#include <boost/test/unit_test.hpp>
#include <iostream>
#include "crest/ConnectionPool.hpp"

BOOST_AUTO_TEST_SUITE(CrestPool)

BOOST_AUTO_TEST_CASE(start_stop)
{
    crest::ConnectionPool pool;
    pool.exit();
}

BOOST_AUTO_TEST_CASE(single_request)
{
    std::cout << "CrestPool::single_request" << std::endl;

    crest::ConnectionPool pool;
    crest::Request request("/market/10000002/orders/buy/?type=https://crest-tq.eveonline.com/inventory/types/34/");
    pool.queue(&request);
    
    auto response = request.wait();
    std::cout << "GET " << request.get_uri_path() << std::endl;
    std::cout << response->status_code << " " << response->status_msg << std::endl;
    std::cout << "Content-Length: " << response->body.size() << std::endl;
    std::cout.write(
        (char*)response->body.data(),
        std::min<size_t>(200, response->body.size()));
    std::cout << std::endl;

    std::cout << "CrestPool::single_request end" << std::endl;
}

BOOST_AUTO_TEST_CASE(bulk_request)
{
    std::cout << "CrestPool::bulk_request" << std::endl;
    std::list<crest::Request> requests;
    crest::ConnectionPool pool;
    for (int i = 0; i < 100; ++i)
    {
        std::string type = "?type=https://crest-tq.eveonline.com/inventory/types/" + std::to_string(34 + i) + "/";
        requests.emplace_back("/market/10000002/orders/buy/" + type);
        pool.queue(&requests.back());
        requests.emplace_back("/market/10000002/orders/sell/" + type);
        pool.queue(&requests.back());
    }
    
    for (auto &req : requests)
    {
        auto response = req.wait();
        std::cout << response->status_code << " " << response->status_msg << " " << req.get_uri_path() << std::endl;
    }
    
    pool.exit();
    std::cout << "CrestPool::bulk_request end" << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()

