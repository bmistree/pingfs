#include <pingfs/ping/echo_request.hpp>

namespace pingfs {

uint16_t EchoRequest::checksum(uint16_t identifier, 
    uint16_t sequence_number, const std::string& body) {
    uint32_t sum = (TYPE << 8) + CODE + htons(identifier)
        + htons(sequence_number);
    for (unsigned int i = 0; i < body.size(); ++i) {
        uint8_t val = static_cast<uint8_t>(body[i]);
        sum += ((i % 2) == 0) ? (val << 8) : val;
    }
    sum += (sum >> 16);
    return ntohs(static_cast<uint16_t>(~sum));
}

EchoRequest::EchoRequest(uint16_t identifier, uint16_t sequence_number,
    const std::string& body)
 : checksum_(checksum(identifier, sequence_number, body)),
   identifier_(identifier),
   sequence_number_(sequence_number),
   body_(body) {
}

EchoRequest::~EchoRequest() {
}

}  // namespace pingfs
