#include <pingfs/ping/echo_request.hpp>
#include <pingfs/ping/echo_response.hpp>
#include <pingfs/ping/ping.hpp>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <functional>

using boost::asio::ip::icmp;

namespace pingfs {

Ping::Ping(boost::asio::io_service* io_service)
  : io_service_(io_service),
    resolver_(*io_service),
    sock_(*io_service, icmp::v4()),
    reply_buffer_() {
    sock_.async_receive(reply_buffer_.prepare(65536),
        std::bind(&Ping::handle_receive, this,
            std::placeholders::_1, std::placeholders::_2));
}

Ping::~Ping() {
}

void Ping::handle_receive(const boost::system::error_code& code,
    std::size_t length) {
    if (code.value() != boost::system::errc::success) {
        // FIXME: Probably should abort for now
        std::cerr << "Error when receiving: " << code << "\n";
    }

    reply_buffer_.commit(length);
    std::istream ipv4_stream(&reply_buffer_);
    IpV4Stream stream(&ipv4_stream);
    EchoResponse echo_response(&stream);
    notify(echo_response);
}

icmp::endpoint Ping::resolve(const std::string& destination) {
    icmp::resolver::query query(icmp::v4(), destination, "");
    return *resolver_.resolve(query);
}

void Ping::ping(const std::string& content,
    const icmp::endpoint& endpoint,
    uint16_t identifier, uint16_t sequence_number) {

    // Generate request
    EchoRequest request(identifier, sequence_number, content);

    // Serialize request
    boost::asio::streambuf request_buffer;
    std::ostream os(&request_buffer);
    os << request;

    // Send request
    sock_.send_to(request_buffer.data(), endpoint);
}


}  // namespace pingfs
