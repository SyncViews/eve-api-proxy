#include <boost/test/unit_test.hpp>
#include "crest/MarketOrderSlim.hpp"
#include <iostream>

using namespace crest;
BOOST_AUTO_TEST_SUITE(TestCrestMarketOrderSlim)

#define CHECK_VEC(a, b) { \
    auto &&a2 = a; \
    auto &&b2 = b; \
    BOOST_CHECK_EQUAL_COLLECTIONS(a2.begin(), a2.end(), b2.begin(), b2.end()); \
}
BOOST_AUTO_TEST_CASE(read)
{
    MarketOrderSlim order;
    json::read_json(
        "{"
        "   \"buy\": true,"
        "   \"duration\": 365,"
        "   \"id\": 911203398,"
        "   \"issued\": \"2016-07-27T21:58:21\","
        "   \"minVolume\": 1,"
        "   \"price\": 120.72,"
        "   \"range\": \"station\","
        "   \"stationID\": 60000382,"
        "   \"type\": 3673,"
        "   \"volume\": 70156,"
        "   \"volumeEntered\": 70376"
        "}"
        , &order);
    BOOST_CHECK_EQUAL(true, order.buy);
    BOOST_CHECK_EQUAL(365, order.duration);
    BOOST_CHECK_EQUAL(911203398, order.id);
    BOOST_CHECK_EQUAL(1469656701, order.issued);
    BOOST_CHECK_EQUAL(1, order.min_volume);
    BOOST_CHECK_CLOSE(120.72, order.price, 0.00001);
    BOOST_CHECK_EQUAL(MarketOrderSlim::RANGE_STATION, order.range);
    BOOST_CHECK_EQUAL(60000382, order.station_id);
    BOOST_CHECK_EQUAL(3673, order.type);
    BOOST_CHECK_EQUAL(70156, order.volume);
    BOOST_CHECK_EQUAL(70376, order.volume_entered);

    json::read_json(
        "{"
        "   \"buy\": true,"
        "   \"duration\": 365,"
        "   \"id\": 911203398,"
        "   \"issued\": \"2016-07-27T21:58:21\","
        "   \"minVolume\": 1,"
        "   \"price\": 120.72,"
        "   \"range\": \"region\","
        "   \"stationID\": 60000382,"
        "   \"type\": 3673,"
        "   \"volume\": 70156,"
        "   \"volumeEntered\": 70376"
        "}"
        , &order);
    BOOST_CHECK_EQUAL(MarketOrderSlim::RANGE_REGION, order.range);

    json::read_json(
        "{"
        "   \"buy\": true,"
        "   \"duration\": 365,"
        "   \"id\": 911203398,"
        "   \"issued\": \"2016-07-27T21:58:21\","
        "   \"minVolume\": 1,"
        "   \"price\": 120.72,"
        "   \"range\": \"3\","
        "   \"stationID\": 60000382,"
        "   \"type\": 3673,"
        "   \"volume\": 70156,"
        "   \"volumeEntered\": 70376"
        "}"
        , &order);
    BOOST_CHECK_EQUAL(3, order.range);
}

BOOST_AUTO_TEST_CASE(write)
{
    MarketOrderSlim order;
    order.buy = false;
    order.id = 911203398;
    order.issued = 1469656701;
    order.min_volume= 2;
    order.price = 120;
    order.range = MarketOrderSlim::RANGE_STATION;
    order.station_id = 60000382;
    order.type = 3673;
    order.duration = 365;
    order.volume = 70156;
    order.volume_entered = 70376;

    auto json = json::to_json(order);
    BOOST_CHECK_EQUAL(
        "{"
        "\"id\":911203398,"
        "\"station_id\":60000382,"
        "\"range\":\"station\","
        "\"issued\":\"2016-07-27T21:58:21Z\","
        "\"buy\":false,"
        "\"price\":120,"
        "\"type\":3673,"
        "\"duration\":365,"
        "\"volume\":70156,"
        "\"min_volume\":2,"
        "\"volume_entered\":70376"
        "}",
        json);

    order.range = MarketOrderSlim::RANGE_REGION;
    json = json::to_json(order);
    BOOST_CHECK_EQUAL(
        "{"
        "\"id\":911203398,"
        "\"station_id\":60000382,"
        "\"range\":\"region\","
        "\"issued\":\"2016-07-27T21:58:21Z\","
        "\"buy\":false,"
        "\"price\":120,"
        "\"type\":3673,"
        "\"duration\":365,"
        "\"volume\":70156,"
        "\"min_volume\":2,"
        "\"volume_entered\":70376"
        "}",
        json);

    order.range = 5;
    json = json::to_json(order);
    BOOST_CHECK_EQUAL(
        "{"
        "\"id\":911203398,"
        "\"station_id\":60000382,"
        "\"range\":\"5\","
        "\"issued\":\"2016-07-27T21:58:21Z\","
        "\"buy\":false,"
        "\"price\":120,"
        "\"type\":3673,"
        "\"duration\":365,"
        "\"volume\":70156,"
        "\"min_volume\":2,"
        "\"volume_entered\":70376"
        "}",
        json);
}

BOOST_AUTO_TEST_SUITE_END()

