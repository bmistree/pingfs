#include <pingfs/ping/spoof/ping_spoof.hpp>

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


void PingSpoof::send(std::shared_ptr<EchoRequest> request,
    const boost::asio::ip::address_v4& spoofed_src,
    const boost::asio::ip::icmp::endpoint& target) {
    // FIXME: Must fill in
}

}  // namespace pingfs
