#include <pingfs/ping/echo_request.hpp>
#include <pingfs/ping/echo_response.hpp>
#include <pingfs/ping/icmp_header.hpp>
#include <pingfs/ping/ip_v4_stream.hpp>
#include <pingfs/ping/ip_v4.hpp>
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
    buffer_index_(0),
    reply_buffer_(60000, 0) {
    set_handler();
}

Ping::~Ping() {
}

void Ping::set_handler() {
    sock_.async_receive(
        boost::asio::buffer(
            &reply_buffer_[buffer_index_],
            reply_buffer_.size() - buffer_index_),
        std::bind(&Ping::handle_receive, this,
            std::placeholders::_1, std::placeholders::_2));
}

bool Ping::check_notify() {
    if (buffer_index_ < IpV4::MIN_SIZE) {
        return false;
    }
    std::string copy(reply_buffer_,
        0 /* pos */,
        buffer_index_);

    std::istringstream ipv4_stream(copy);
    IpV4Stream stream(&ipv4_stream);
    IpV4 ip_v4(&stream, buffer_index_);

    if (ip_v4.get_length() > buffer_index_) {
        // More bytes to read for this packet
        return false;
    }

    // Copy data forward
    // FIXME: Probably should use a ring buffer instead
    // of copying data
    reply_buffer_.replace(0, copy.size(), copy);
    buffer_index_ -= ip_v4.get_length();

    // We will take bytes from this buffer and
    // try to produce a response.
    if (!ip_v4.is_icmp()) {
        // This wasn't an ICMP packet (for some reason).
        return true;
    }

    IcmpHeader icmp_header(&stream);

    if (icmp_header.get_code() != EchoResponse::CODE) {
        return true;
    }
    if (icmp_header.get_type() != EchoResponse::TYPE) {
        return true;
    }

    std::vector<char> data_buffer(
        ip_v4.get_length() -
        // Size of IP header; FIXME: we're assuming
        // this is fixed
        IpV4::MIN_SIZE -
        // Size of ICMP header
        // FIXME: do not hard code
        8);
    ipv4_stream.read(&data_buffer[0], data_buffer.size());

    std::string data(data_buffer.begin(), data_buffer.end());
    EchoResponse echo_response(
        icmp_header.get_identifier(),
        icmp_header.get_sequence_number(),
        data);
    notify(echo_response);
    return true;
}

void Ping::handle_receive(
    const boost::system::error_code& code,
    std::size_t length) {
    buffer_index_ += length;
    if (code.value() != boost::system::errc::success) {
        // FIXME: Probably should abort for now
        std::cerr << "Error when receiving: " << code << "\n";
        throw "Unexpected error";
    }

    while (check_notify()) {
    }
    set_handler();
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
    ping(request, endpoint);
}

void Ping::ping(const EchoRequest& request,
    const icmp::endpoint& endpoint) {
    // Serialize request
    boost::asio::streambuf request_buffer;
    std::ostream os(&request_buffer);
    os << request;

    // Send request
    sock_.send_to(request_buffer.data(), endpoint);
}

}  // namespace pingfs
