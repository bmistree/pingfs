#ifndef _ECHO_REQUEST_
#define _ECHO_REQUEST_

#include <string>

namespace pingfs {

class EchoRequest {
public:
    EchoRequest(uint16_t identifier, uint16_t sequence_number,
        const std::string& body);
    ~EchoRequest();

private:
    const uint16_t checksum_;
    const uint16_t identifier_;
    const uint16_t sequence_number_;
    const std::string& body_;
    
    static const uint8_t TYPE = 8;
    static const uint8_t CODE = 0;
    
    static uint16_t checksum(uint16_t identifier, 
        uint16_t sequence_number, const std::string& body);

};


}  // namespace pingfs

#endif 

