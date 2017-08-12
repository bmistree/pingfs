#include <pingfs/ping/echo_request.hpp>

namespace pingfs {

EchoRequest::EchoRequest(uint16_t identifier, uint16_t sequence_number,
    const std::string& body)
 : identifier_(identifier),
   sequence_number_(sequence_number),
   body_(body) {
}

EchoRequest::~EchoRequest() {
}

}  // namespace pingfs
