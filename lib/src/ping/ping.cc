#include <pingfs/ping/echo_request.hpp>
#include <pingfs/ping/ping.hpp>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::icmp;

namespace pingfs {

static const uint16_t IDENTIFIER = 3353;
static const uint16_t SEQUENCE_NUMBER = 1111;

std::string ping(const std::string& content, const std::string& destination,
    boost::asio::io_service& io_service) {
    icmp::resolver::query query(icmp::v4(), destination, "");

    icmp::resolver resolver(io_service);
    icmp::socket sock(io_service, icmp::v4());
    icmp::endpoint endpoint = *resolver.resolve(query);

    // Generate request
    EchoRequest request(IDENTIFIER, SEQUENCE_NUMBER, content);

    // Serialize request
    boost::asio::streambuf request_buffer;
    std::ostream os(&request_buffer);
    os << request;

    // Send request
    sock.send_to(request_buffer.data(), endpoint);
    return "dummy ping";
}


}  // namespace pingfs
