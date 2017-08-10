#include <pingfs/ping/ping.hpp>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::icmp;

namespace pingfs {

std::string ping(const std::string& content, const std::string& destination,
        const boost::asio::io_service& io_service) {
    icmp::resolver::query query(icmp::v4(), destination, "");
    throw "Incomplete";
}


}  // namespace pingfs
