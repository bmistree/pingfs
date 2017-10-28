#ifndef _ECHO_RESPONSE_
#define _ECHO_RESPONSE_

#include <boost/asio.hpp>

#include <string>

#include "ip_v4_stream.hpp"

namespace pingfs {

class EchoResponse {
 public:
    explicit EchoResponse(IpV4Stream* ipv4_stream);
    EchoResponse(uint16_t identifier,
        uint16_t sequence_number,
        const std::string data);

    ~EchoResponse();

    const std::string& get_data() const;
    const uint16_t get_identifier() const;
    const uint16_t get_sequence_number() const;

 private:
    const uint16_t identifier_;
    const uint16_t sequence_number_;
    const std::string data_;
};


}  // namespace pingfs

#endif
