#ifndef _IP_V4_
#define _IP_V4_

#include <iostream>
#include <string>

#include "ip_v4_stream.hpp"

namespace pingfs {

class IpV4 {
 public:
    static const std::size_t MIN_SIZE = 20;

    IpV4(IpV4Stream* istream, std::size_t length);
    ~IpV4();

    bool is_icmp() const;
    uint16_t get_length() const;

 private:
    const uint16_t length_;
    const uint8_t proto_;

    static uint16_t get_total_length(IpV4Stream* istream);
    static uint8_t get_proto(IpV4Stream* istream);

};

}  // namespace pingfs

#endif
