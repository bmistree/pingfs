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
    const uint16_t identifier_;
    const uint16_t sequence_number_;
    const std::string& body_;
    
};


}  // namespace pingfs

#endif 

