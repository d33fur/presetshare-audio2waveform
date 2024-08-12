#include "server.hpp"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: audio2waveform <address> <port>\n"
              << "Example:\n"
              << "    audio2waveform 0.0.0.0 8080\n";
    return EXIT_FAILURE;
  }

  auto const address = net::ip::make_address(argv[1]);
  auto const port = static_cast<unsigned short>(std::atoi(argv[2]));

  net::io_context ioc{1};

  std::make_shared<listener>(ioc, tcp::endpoint{address, port})->run();

  ioc.run();

  return EXIT_SUCCESS;
}