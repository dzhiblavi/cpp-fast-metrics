#ifndef LIB_FAST_METRICS_PULL_SERVICE_HPP
#define LIB_FAST_METRICS_PULL_SERVICE_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio.hpp>
#include <boost/config.hpp>

#include <atomic>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "src/fast_metrics/Metrics.hpp"

namespace fmetrics::http {

class PullService {
 public:
  explicit PullService(Metrics& metrics);

  void Start(unsigned short port);

  void Shutdown();

  void Wait();

 private:
  void StartAccept(boost::asio::ip::tcp::acceptor& acceptor);

  void HandleAccept(
      const boost::system::error_code& error,
      std::shared_ptr<boost::asio::ip::tcp::socket> socket,
      boost::asio::ip::tcp::acceptor& acceptor);

  Metrics& metrics_;
  boost::asio::io_context service_;
	std::atomic<bool> stop_{false};
  std::thread thread_;
};

}  // namespace fmetrics::http

#endif  // LIB_FAST_METRICS_PULL_SERVICE_HPP
