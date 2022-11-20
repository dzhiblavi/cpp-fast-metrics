#include "src/fast_metrics/http/PullService.hpp"

#include <boost/bind.hpp>
#include <boost/exception/diagnostic_information.hpp>

namespace {

template <class Body, class Allocator, class Send>
void handle_request(
    boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>&& req,
    Send&& send, fmetrics::Metrics& metrics) {
  auto const bad_request = [&req](boost::beast::string_view why) {
    boost::beast::http::response<boost::beast::http::string_body> res{
        boost::beast::http::status::bad_request, req.version()};
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    return res;
  };
  auto const server_error = [&req](boost::beast::string_view what) {
    boost::beast::http::response<boost::beast::http::string_body> res{
        boost::beast::http::status::internal_server_error, req.version()};
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "An error occurred: '" + std::string(what) + "'";
    res.prepare_payload();
    return res;
  };

  try {
    if (req.method() != boost::beast::http::verb::get || req.target() != "/solomon") {
      return send(bad_request("Invalid argument"));
    } else {
      boost::beast::http::response<boost::beast::http::string_body> res{
          boost::beast::http::status::internal_server_error, req.version()};
      res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
      res.set(boost::beast::http::field::content_type, "application/json");
      std::stringstream response_stream;
      metrics.Dump(response_stream);
      std::string response = response_stream.str();
      res.body() = response;
      res.content_length(response.size());
      res.keep_alive(req.keep_alive());
      res.result(200);
      return send(std::move(res));
    }
  } catch (const boost::exception& exc) {
    return send(server_error("Failed to handle request: " + boost::diagnostic_information(&exc)));
  }
}

void fail(boost::beast::error_code ec, char const* what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

template <class Stream>
struct send_lambda {
  Stream& stream_;
  bool& close_;
  boost::beast::error_code& ec_;

  explicit send_lambda(Stream& stream, bool& close, boost::beast::error_code& ec)
      : stream_(stream), close_(close), ec_(ec) {}

  template <bool isRequest, class Body, class Fields>
  void operator()(boost::beast::http::message<isRequest, Body, Fields>&& msg) const {
    boost::beast::http::serializer<isRequest, Body, Fields> sr{msg};
    boost::beast::http::write(stream_, sr, ec_);
  }
};

void do_session(boost::asio::ip::tcp::socket& socket, fmetrics::Metrics& metrics) {
  bool close = false;
  boost::beast::error_code ec;
  boost::beast::flat_buffer buffer;
  send_lambda<boost::asio::ip::tcp::socket> lambda{socket, close, ec};

  for (;;) {
    boost::beast::http::request<boost::beast::http::string_body> req;
    boost::beast::http::read(socket, buffer, req, ec);
    if (ec == boost::beast::http::error::end_of_stream) {
      break;
    }
    if (ec) {
      return fail(ec, "read");
    }
    handle_request(std::move(req), lambda, metrics);
    if (ec) {
      return fail(ec, "write");
    }
    break;
  }

  socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
}

}  // namespace

namespace fmetrics::http {

PullService::PullService(Metrics& metrics) : metrics_(metrics) {}

void PullService::StartAccept(boost::asio::ip::tcp::acceptor& acceptor) {
  std::shared_ptr<boost::asio::ip::tcp::socket> socket(
      new boost::asio::ip::tcp::socket(service_));

  acceptor.async_accept(
      *socket,
      boost::bind(
        &PullService::HandleAccept, this,
        boost::asio::placeholders::error,
        socket, boost::ref(acceptor)));
}

void PullService::HandleAccept(
    const boost::system::error_code& error,
    std::shared_ptr<boost::asio::ip::tcp::socket> socket,
    boost::asio::ip::tcp::acceptor& acceptor) {

  if (error) {
    return;
  }

  do_session(*socket, metrics_);
  StartAccept(acceptor);
}

void PullService::Start(unsigned short port) {
  thread_ = std::thread([this, port] {
    while (!stop_.load(std::memory_order_relaxed)) {
      boost::asio::ip::tcp::acceptor acceptor(
          service_,
          { boost::asio::ip::address::from_string("::").to_v6(), port });
      boost::asio::socket_base::reuse_address reuse_address_opt(true);
      acceptor.set_option(reuse_address_opt);
      StartAccept(acceptor);
      service_.run();
    }
  });
}

void PullService::Shutdown() {
  service_.stop();
  stop_.store(true, std::memory_order_relaxed);
}

void PullService::Wait() { thread_.join(); }

}  // namespace fmetrics::http
