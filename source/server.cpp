#include "processing.hpp"
#include "server.hpp"
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/config.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <vector>

#define SERVER_VERSION "waveform server"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

void fail(beast::error_code ec, char const *what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

session::session(tcp::socket &&socket) : stream_(std::move(socket)) {}

void session::run() {
  net::dispatch(
      stream_.get_executor(),
      beast::bind_front_handler(&session::do_read, shared_from_this()));
}

void session::do_read() {
  req_ = {};

  parser_.emplace(std::piecewise_construct, std::make_tuple(),
                  std::make_tuple());

  parser_->body_limit(15 * 1024 * 1024);

  stream_.expires_after(std::chrono::seconds(15));

  http::async_read(
      stream_, buffer_, *parser_,
      beast::bind_front_handler(&session::on_read, shared_from_this()));
}

void session::on_read(beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if(ec == http::error::end_of_stream)
    return do_close();

  if(ec)
    return fail(ec, "read");

  req_ = parser_->release();

  if(req_.method() == http::verb::post && req_.target() == "/waveform") {
    // auto content_type = req_[http::field::content_type];
    // if(content_type != "audio/wave") {
    //   http::response<http::string_body> res{http::status::unsupported_media_type, req_.version()};
    //   res.set(http::field::server, SERVER_VERSION);
    //   res.set(http::field::content_type, "text/plain");
    //   res.body() = "Unsupported Media Type: Expected audio/wave";
    //   res.content_length(res.body().size());
    //   res.keep_alive(req_.keep_alive());

    //   send_response(std::move(res));
    //   return;
    // }
    try {
      int input_width = std::stoi(req_.at("input-w"));
      int input_height = std::stoi(req_.at("input-h"));
      int output_width = std::stoi(req_.at("output-w"));
      int output_height = std::stoi(req_.at("output-h"));
      bool filled = req_.at("filled") == "true";
      float scale = std::stof(req_.at("scale"));
      int line = std::stoi(req_.at("line"));

      const std::string filename = "uploaded_audio.wav";
      std::ofstream ofs(filename, std::ios::binary);
      ofs << req_.body();
      ofs.close();

      std::vector<float> data;
      int sampleRate;
      readAudioData(filename, data, sampleRate);

      const std::string output_image = "waveform.webp";

      createWaveformImage(data, output_image, input_width, input_height, output_width, output_height, filled, scale, line);

      std::ifstream ifs(output_image, std::ios::binary);
      std::vector<char> image_data((std::istreambuf_iterator<char>(ifs)),
                                    std::istreambuf_iterator<char>());

      http::response<http::vector_body<char>> res{http::status::ok, req_.version()};
      res.set(http::field::server, SERVER_VERSION);
      res.set(http::field::content_type, "image/webp");
      res.body() = std::move(image_data);
      res.content_length(res.body().size());
      res.keep_alive(req_.keep_alive());

      send_response(std::move(res));
    }
    catch(const std::exception& err) {
      std::cout << "err" << std::endl;

      http::response<http::string_body> res{http::status::bad_request, req_.version()};
      res.set(http::field::server, SERVER_VERSION);
      res.set(http::field::content_type, "text/html");
      res.keep_alive(req_.keep_alive());
      res.body() = "Invalid request method or path";
      res.prepare_payload();
      send_response(std::move(res));
    }
  } else {
    http::response<http::string_body> res{http::status::bad_request, req_.version()};
    res.set(http::field::server, SERVER_VERSION);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req_.keep_alive());
    res.body() = "Invalid request method or path";
    res.prepare_payload();
    send_response(std::move(res));
  }
}

void session::send_response(http::message_generator &&msg) {
  bool keep_alive = msg.keep_alive();

  beast::async_write(stream_, std::move(msg),
                     beast::bind_front_handler(&session::on_write,
                                               shared_from_this(), keep_alive));
}

void session::on_write(bool keep_alive, beast::error_code ec,
                       std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec)
    return fail(ec, "write");

  if (!keep_alive) {
    return do_close();
  }

  do_read();
}

void session::do_close() {
  beast::error_code ec;
  stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
}

listener::listener(net::io_context &ioc, tcp::endpoint endpoint)
    : ioc_(ioc), acceptor_(net::make_strand(ioc)) {
  beast::error_code ec;

  acceptor_.open(endpoint.protocol(), ec);
  if (ec) {
    fail(ec, "open");
    return;
  }

  acceptor_.set_option(net::socket_base::reuse_address(true), ec);
  if (ec) {
    fail(ec, "set_option");
    return;
  }

  acceptor_.bind(endpoint, ec);
  if (ec) {
    fail(ec, "bind");
    return;
  }

  acceptor_.listen(net::socket_base::max_listen_connections, ec);
  if (ec) {
    fail(ec, "listen");
    return;
  }
}

void listener::run() { do_accept(); }

void listener::do_accept() {
  acceptor_.async_accept(
      net::make_strand(ioc_),
      beast::bind_front_handler(&listener::on_accept, shared_from_this()));
}

void listener::on_accept(beast::error_code ec, tcp::socket socket) {
  if (ec) {
    fail(ec, "accept");
  } else {
    std::make_shared<session>(std::move(socket))->run();
  }

  do_accept();
}
