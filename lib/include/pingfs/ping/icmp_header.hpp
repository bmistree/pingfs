#ifndef _ICMP_HEADER_
#define _ICMP_HEADER_

#include "ip_v4_stream.hpp"

namespace pingfs {

class IcmpHeader {
 public:
    /**
     * @param istream Should already be advanced past IP
     * header.
     */
    explicit IcmpHeader(IpV4Stream* istream);
    ~IcmpHeader();

    uint8_t get_type() const;
    uint8_t get_code() const;
    uint16_t get_identifier() const;
    uint16_t get_sequence_number() const;

 private:
    uint8_t type_;
    uint8_t code_;
    uint16_t checksum_;
    uint16_t identifier_;
    uint16_t sequence_number_;
};

}  // namespace pingfs

#endif
