#ifndef _SPOOFED_IP_V4_
#define _SPOOFED_IP_V4_

#include <boost/asio.hpp>
#include <pingfs/ping/echo_request.hpp>

#include <memory>
#include <ostream>

namespace pingfs {

class SpoofedIpV4 {

 public:
    SpoofedIpV4(boost::asio::ip::address_v4 src,
        boost::asio::ip::address_v4 dst,
        const EchoRequest& request);
    ~SpoofedIpV4();

    friend std::ostream& operator<< (std::ostream& os,
        const SpoofedIpV4& request);

    // not setting any options: this header's length is 20 bytes (5 * 4)
    const static uint8_t IP_HEADER_LENGTH_BYTES = 20;
    const static uint8_t IP_HEADER_LENGTH = 5;
    const static uint8_t IPV4_VERSION = 4;
    const static uint8_t IP_PROTOCOL_ICMP = 1;

 private:
    boost::asio::ip::address_v4 src_;
    boost::asio::ip::address_v4 dst_;
    const EchoRequest& request_;
};


}  // namespace pingfs

#endif
