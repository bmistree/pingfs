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
    reply_buffer_(60000, 0),
    data_buffer_(),
    data_buffer_read_mutex_() {
    set_handler();
}

Ping::~Ping() {
}

void Ping::set_handler() {
    sock_.async_receive(
        boost::asio::buffer(
            &reply_buffer_[0],
            reply_buffer_.size()),
        std::bind(&Ping::handle_receive, this,
            std::placeholders::_1, std::placeholders::_2));
}

bool Ping::check_notify() {
    boost::mutex::scoped_lock locker(data_buffer_read_mutex_);
    std::string copy = data_buffer_.peek();
    if (copy.size() < IpV4::MIN_SIZE) {
        return false;
    }

    std::istringstream ipv4_stream(copy);
    IpV4Stream stream(&ipv4_stream);
    IpV4 ip_v4(&stream, copy.size());
    
    if (ip_v4.get_length() > copy.size()) {
        // More bytes to read for this packet
        return false;
    }
    data_buffer_.discard(ip_v4.get_length());

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
    io_service_->post(std::bind(&Ping::notify, this, echo_response));
    return true;
}

void Ping::handle_receive(
    const boost::system::error_code& code,
    std::size_t length) {
    if (code.value() != boost::system::errc::success) {
        // FIXME: Probably should abort for now
        std::cerr << "Error when receiving: " << code;
        throw "Unexpected error";
    }
    std::string data(reply_buffer_, 0 /* pos */, length);
    set_handler();

    std::string content(data, 0, length);
    data_buffer_.append(content);
    io_service_->post(std::bind(&Ping::internal_check_notify, this));
}

void Ping::internal_check_notify() {
    while (check_notify() && (data_buffer_.size() != 0)) {
    }
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
