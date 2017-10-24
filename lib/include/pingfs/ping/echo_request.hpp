#ifndef _ECHO_REQUEST_
#define _ECHO_REQUEST_

#include <boost/asio.hpp>
#include <string>

namespace pingfs {

class EchoRequest {
 public:
    EchoRequest(uint16_t identifier, uint16_t sequence_number,
        const std::string& body);
    ~EchoRequest();

    friend std::ostream& operator<< (std::ostream& os,
        const EchoRequest& request);

    uint16_t get_identifier() const;
    uint16_t get_sequence_number() const;
    const std::string& get_body() const;

 private:
    const uint16_t checksum_;
    const uint16_t identifier_;
    const uint16_t sequence_number_;
    // FIXME: we copy a string in here; this may be expensive
    const std::string body_;

    static const uint8_t TYPE = 8;
    static const uint8_t CODE = 0;

    static uint16_t checksum(uint16_t identifier,
        uint16_t sequence_number, const std::string& body);
};

}  // namespace pingfs

#endif
