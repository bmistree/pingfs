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
    reply_buffer_() {
    sock_.async_receive(reply_buffer_.prepare(65536),
        std::bind(&Ping::handle_receive, this,
            std::placeholders::_1, std::placeholders::_2));
}

Ping::~Ping() {
}

void Ping::handle_receive(const boost::system::error_code& code,
    std::size_t length) {
    sock_.async_receive(reply_buffer_.prepare(65536),
        std::bind(&Ping::handle_receive, this,
            std::placeholders::_1, std::placeholders::_2));


    // FIXME: Should probably check that:
    //  1) This is an ICMP response;
    //     (spent a while debugging when was issuing
    //     pings to localhost why I got duplicate
    //     messages.
    //  2) We received all bytes for this request
    if (code.value() != boost::system::errc::success) {
        // FIXME: Probably should abort for now
        std::cerr << "Error when receiving: " << code << "\n";
    }

    if (length < IpV4::MIN_SIZE) {
        return;
    }
    std::istream ipv4_stream(&reply_buffer_);
    IpV4Stream stream(&ipv4_stream);
    IpV4 ip_v4(&stream, length);

    if (ip_v4.get_length() > length) {
        // More bytes to read for this packet
        return;
    }

    // We will take bytes from this buffer and
    // try to produce a response.
    reply_buffer_.commit(ip_v4.get_length());

    if (!ip_v4.is_icmp()) {
        // This wasn't an ICMP packet (for some reason).
        return;
    }

    IcmpHeader icmp_header(&stream);

    if (icmp_header.get_code() != EchoResponse::CODE) {
        return;
    }
    if (icmp_header.get_type() != EchoResponse::TYPE) {
        return;
    }

    std::vector<char> data_buffer(
        ip_v4.get_length() -
        // Size of IP header; FIXME: we're assuming
        // this is fixed
        20 -
        // Size of ICMP header
        8);
    ipv4_stream.read(&data_buffer[0], data_buffer.size());

    std::string data(data_buffer.begin(), data_buffer.end());
    EchoResponse echo_response(
        icmp_header.get_identifier(),
        icmp_header.get_sequence_number(),
        data);
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
