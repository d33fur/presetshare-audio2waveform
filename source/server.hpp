#ifndef SERVER_HPP
#define SERVER_HPP

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <memory>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class session : public std::enable_shared_from_this<session> {
public:
  session(tcp::socket &&socket);
  void run();

private:
  void do_read();
  void on_read(beast::error_code ec, std::size_t bytes_transferred);
  void send_response(http::message_generator &&msg);
  void on_write(bool keep_alive, beast::error_code ec,
                std::size_t bytes_transferred);
  void do_close();

  beast::tcp_stream stream_;
  beast::flat_buffer buffer_;
  http::request<http::string_body> req_;
  std::optional<http::request_parser<http::string_body>> parser_;
};

class listener : public std::enable_shared_from_this<listener> {
public:
  listener(net::io_context &ioc, tcp::endpoint endpoint);
  void run();

private:
  void do_accept();
  void on_accept(beast::error_code ec, tcp::socket socket);

  net::io_context &ioc_;
  tcp::acceptor acceptor_;
};

void fail(beast::error_code ec, char const *what);

#endif // SERVER_HPP
