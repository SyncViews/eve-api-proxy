#include <boost/test/unit_test.hpp>
#include "socket/TlsSocket.hpp"
#include "Error.hpp"
#include <iostream>

BOOST_AUTO_TEST_SUITE(socket_TlsSocket)

BOOST_AUTO_TEST_CASE(connect)
{
    std::cout << "GET https://willnewbery.co.uk:443/" << std::endl;
    const char *HTTP_REQ =
        "GET / HTTP/1.0\r\n"
        "Host: willnewbery.co.uk\r\n"
        "\r\n";
    TlsSocket sock;
    sock.connect("willnewbery.co.uk", 443);
    sock.send_all((const uint8_t*)HTTP_REQ, strlen(HTTP_REQ));

    char buffer[4096];
    while (auto len = sock.recv((uint8_t*)buffer, sizeof(buffer)))
    {
        std::cout.write(buffer, len);
        std::cout.flush();
    }
    std::cout << std::endl << std::endl; 
}

BOOST_AUTO_TEST_SUITE_END()
