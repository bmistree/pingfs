#include <pingfs/ping/echo_request.hpp>
#include <pingfs/ping/ping.hpp>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::icmp;

namespace pingfs {

Ping::Ping(boost::asio::io_service& io_service)
 : io_service_(io_service),
   resolver_(io_service),
   sock_(io_service, icmp::v4()) {
}

Ping::~Ping() {
}

void Ping::ping(const std::string& content, const std::string& destination,
    uint16_t identifier, uint16_t sequence_number) {
    icmp::resolver::query query(icmp::v4(), destination, "");
    icmp::endpoint endpoint = *resolver_.resolve(query);

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
