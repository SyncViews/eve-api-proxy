#include <boost/test/unit_test.hpp>
#include "socket/Socket.hpp"
#include "Error.hpp"
#include <iostream>

BOOST_AUTO_TEST_SUITE(socket_Socket)

BOOST_AUTO_TEST_CASE(invalid_host)
{
    Socket sock;
    BOOST_CHECK_THROW(sock.connect("not#a@valid@domain", 80), NetworkError);
}

//TODO: this is more difficult to test. Some annoying ISP's like BT like to return a "fake" DNS record to insert there own web pages...
/*BOOST_AUTO_TEST_CASE(unknown_host)
{
    Socket sock;
    BOOST_CHECK_THROW(sock.connect("does-not-exist.willnewbery.co.uk", 80), NetworkError);
}*/

BOOST_AUTO_TEST_CASE(no_connect)
{
    Socket sock;
    BOOST_CHECK_THROW(sock.connect("willnewbery.co.uk", 3432), NetworkError);
}

BOOST_AUTO_TEST_CASE(connect)
{
    const char *HTTP_REQ =
        "GET / HTTP/1.0\r\n"
        "Host: willnewbery.co.uk\r\n"
        "\r\n";
    Socket sock;
    sock.connect("willnewbery.co.uk", 80);
    sock.send_all((const uint8_t*)HTTP_REQ, strlen(HTTP_REQ));

    char buffer[4096];
    while (auto len = sock.recv((uint8_t*)buffer, sizeof(buffer)))
    {
        std::cout.write(buffer, len);
    }
}

BOOST_AUTO_TEST_SUITE_END()
