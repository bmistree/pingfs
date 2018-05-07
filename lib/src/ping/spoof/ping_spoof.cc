#include <pingfs/ping/spoof/ping_spoof.hpp>
#include <pingfs/ping/spoof/spoofed_ip_v4.hpp>

#include <boost/asio.hpp>

#include <sys/socket.h>

namespace pingfs {


PingSpoof::PingSpoof(boost::asio::io_service* io_service)
 : io_service_(io_service),
   resolver_(*io_service),
   sock_(*io_service, boost::asio::ip::icmp::v4()) {

    int on = 1;
    // We will include ip header information as part of sending pings
    setsockopt(
        sock_.native(),
        IPPROTO_IP,
        IP_HDRINCL,
        (const char*)&on,
        sizeof (on));
}


PingSpoof::~PingSpoof() {
}


void PingSpoof::send(
    const std::string& content,
    uint16_t identifier,
    uint16_t sequence_number,
    const boost::asio::ip::address_v4& spoofed_src,
    const boost::asio::ip::address_v4& spoofed_target) {

    boost::asio::streambuf request_buffer;
    std::ostream os(&request_buffer);

    EchoRequest echo_request(identifier, sequence_number, content);
    SpoofedIpV4 spoofed(spoofed_src, spoofed_target, echo_request);
    os << spoofed;

    boost::asio::ip::icmp::endpoint endpoint(
        spoofed_target,
        // Filling in dummy port number here for endpoint.
        // ICMP doesn't need a port number, but boost
        // doesn't seem happy to run without it.
        1);

    // Send the spoofed packet
    sock_.send_to(request_buffer.data(), endpoint);
}

}  // namespace pingfs
