#include "server.hpp"
#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: server <address> <port>\n";
        return EXIT_FAILURE;
    }
    auto const address = net::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));

    net::io_context ioc{1};

    std::make_shared<listener>(ioc, tcp::endpoint{address, port})->run();

    ioc.run();

    return EXIT_SUCCESS;
}
