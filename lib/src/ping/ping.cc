#include <pingfs/ping/echo_request.hpp>
#include <pingfs/ping/ping.hpp>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::icmp;

namespace pingfs {

Ping::Ping(boost::asio::io_service& io_service)
 : io_service_(io_service) {
}

Ping::~Ping() {
}


std::string Ping::ping(const std::string& content, const std::string& destination,
    uint16_t identifier, uint16_t sequence_number) {
    icmp::resolver::query query(icmp::v4(), destination, "");

    icmp::resolver resolver(io_service_);
    icmp::socket sock(io_service_, icmp::v4());
    icmp::endpoint endpoint = *resolver.resolve(query);

    // Generate request
    EchoRequest request(identifier, sequence_number, content);

    // Serialize request
    boost::asio::streambuf request_buffer;
    std::ostream os(&request_buffer);
    os << request;

    // Send request
    sock.send_to(request_buffer.data(), endpoint);
    return "dummy ping";
}


}  // namespace pingfs
