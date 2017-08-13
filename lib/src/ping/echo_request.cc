#include <pingfs/ping/echo_request.hpp>

namespace pingfs {

uint16_t EchoRequest::checksum(uint16_t identifier, 
    uint16_t sequence_number, const std::string& body) {
    // Note that we do *not* need htons here because we're doing a
    // mathematical operation on the data, whose value shouldn't
    // depend on endianness.
    uint32_t sum = (TYPE << 8) + CODE + identifier + sequence_number;
    for (unsigned int i = 0; i < body.size(); ++i) {
        uint8_t val = static_cast<uint8_t>(body[i]);
        sum += ((i % 2) == 0) ? (val << 8) : val;
    }
    sum += (sum >> 16);
    // Note that we do need ntohs here because, by definition, 
    // the value that we generate will be in network order.
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

static void put_short(std::ostream&os, uint16_t s) {
    os.put(static_cast<unsigned char>(s >> 8));
    os.put(static_cast<unsigned char>(s & 0x00FF));
}

std::ostream& operator<< (std::ostream& os, const EchoRequest& request) {
    os.put(static_cast<char>(EchoRequest::TYPE));
    os.put(static_cast<char>(EchoRequest::CODE));
    put_short(os, htons(request.checksum_));
    put_short(os, request.identifier_);
    put_short(os, request.sequence_number_);
    os << request.body_;
    return os;
}

}  // namespace pingfs
