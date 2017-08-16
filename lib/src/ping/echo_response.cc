#include <pingfs/ping/echo_response.hpp>

namespace pingfs {


EchoResponse::EchoResponse(IpV4Stream& ipv4_stream)
  : identifier_(ipv4_stream.consume_ip_header()
      // type (1B), code (1B), checksum (2B)
      // FIXME: we should maybe check the checksum here.
      .read_bytes(4)
      // actual identifier
      .read_unsigned_short()),
    sequence_number_(ipv4_stream.read_unsigned_short()),
    data_(ipv4_stream.to_str()) {
}

EchoResponse::EchoResponse(uint16_t identifier, uint16_t sequence_number,
    const std::string data)
  : identifier_(identifier),
    sequence_number_(sequence_number),
    data_(data) {
}

const std::string& EchoResponse::get_data() const {
    return data_;
}

const uint16_t EchoResponse::get_identifier() const {
    return identifier_;
}

const uint16_t EchoResponse::get_sequence_number() const {
    return sequence_number_;
}

EchoResponse::~EchoResponse() {
}


}  // namespace pingfs
