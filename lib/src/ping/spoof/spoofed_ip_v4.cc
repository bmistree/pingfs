#include <pingfs/ping/spoof/spoofed_ip_v4.hpp>

#include <linux/ip.h>
#include <unistd.h>

namespace pingfs {

SpoofedIpV4::SpoofedIpV4(boost::asio::ip::address_v4 src,
    boost::asio::ip::address_v4 dst,
    const EchoRequest& request)
 : src_(src),
   dst_(dst),
   request_(request) {
}

SpoofedIpV4::~SpoofedIpV4() {
}

/**
 * Returns in network order
 */
static uint32_t boost_addr_to_binary(const boost::asio::ip::address_v4& addr) {
    boost::asio::ip::address_v4::bytes_type bytes = addr.to_bytes();
    uint32_t value = 0;
    value |= bytes[3] << 24;
    value |= bytes[2] << 16;
    value |= bytes[1] << 8;
    value |= bytes[0];
    return value;
}

std::ostream& operator<< (std::ostream& os, const SpoofedIpV4& hdr) {
    iphdr ip_hdr;

    ip_hdr.ihl = SpoofedIpV4::IP_HEADER_LENGTH;
    ip_hdr.version = SpoofedIpV4::IPV4_VERSION;
    ip_hdr.tot_len = sizeof(struct iphdr) + hdr.request_.byte_size();
    ip_hdr.protocol = SpoofedIpV4::IP_PROTOCOL_ICMP;
    // Setting to arbitrary value
    ip_hdr.ttl = 148;
    ip_hdr.saddr = boost_addr_to_binary(hdr.src_);
    ip_hdr.daddr = boost_addr_to_binary(hdr.dst_);
    // This will be offloaded to hardware (hopefully)
    ip_hdr.check = 0;

    os.write(
        reinterpret_cast<const char*>(&ip_hdr),
        SpoofedIpV4::IP_HEADER_LENGTH_BYTES);
    os << hdr.request_;
    return os;
}


}  // namespace pingfs
