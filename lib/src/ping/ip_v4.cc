
#include <pingfs/ping/ip_v4.hpp>

namespace pingfs {

uint16_t IpV4::get_total_length(IpV4Stream* istream) {
    // version, ihl, differentiated services
    istream->read_unsigned_short();
    return istream->read_unsigned_short();
}

uint8_t IpV4::get_proto(IpV4Stream* istream) {
    // identification
    istream->read_unsigned_short();
    // flags, fragment offset
    istream->read_unsigned_short();
    // TTL
    istream->read_byte();
    // protocol
    return istream->read_byte();
}

IpV4::IpV4(IpV4Stream* istream, std::size_t length)
  : length_(get_total_length(istream)),
    proto_(get_proto(istream)) {
    if (length < MIN_SIZE) {
        throw "Cannot deserialize header; too short";
    }
}

IpV4::~IpV4() {
}

bool IpV4::is_icmp() const {
    return proto_ == 1;
}

uint16_t IpV4::get_length() const {
    return length_;
}

}  // namespace pingfs
