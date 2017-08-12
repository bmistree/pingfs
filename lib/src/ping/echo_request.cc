#include <pingfs/ping/echo_request.hpp>

namespace pingfs {

uint16_t checksum(uint16_t identifier, uint16_t sequence_number,
    const std::string& body) {
    uint16_t sum = (header.type() << 8) + header.code()
        + header.identifier() + header.sequence_number();

    Iterator body_iter = body_begin;
    while (body_iter != body_end)
    {
        sum += (static_cast<unsigned char>(*body_iter++) << 8);
        if (body_iter != body_end)
            sum += static_cast<unsigned char>(*body_iter++);
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
    header.checksum(static_cast<unsigned short>(~sum));
}

EchoRequest::EchoRequest(uint16_t identifier, uint16_t sequence_number,
    const std::string& body)
 : checksum_(),
   identifier_(identifier),
   sequence_number_(sequence_number),
   body_(body) {
}

EchoRequest::~EchoRequest() {
}

}  // namespace pingfs
