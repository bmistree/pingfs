#include <pingfs/ping/icmp_header.hpp>

namespace pingfs {

IcmpHeader::IcmpHeader(IpV4Stream* istream)
  : type_(istream->read_byte()),
    code_(istream->read_byte()),
    checksum_(istream->read_unsigned_short()),
    identifier_(istream->read_unsigned_short()),
    sequence_number_(istream->read_unsigned_short()) {
}

IcmpHeader::~IcmpHeader() {
}

uint8_t IcmpHeader::get_type() const {
    return type_;
}

uint8_t IcmpHeader::get_code() const {
    return code_;
}

uint16_t IcmpHeader::get_identifier() const {
    return identifier_;
}

uint16_t IcmpHeader::get_sequence_number() const {
    return sequence_number_;
}

}  // namespace pingfs
