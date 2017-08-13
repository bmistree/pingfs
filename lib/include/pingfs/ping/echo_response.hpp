#ifndef _ECHO_RESPONSE_
#define _ECHO_RESPONSE_

#include <boost/asio.hpp>

#include <iostream>
#include <string>

namespace pingfs {

class EchoResponse;

class IpV4Stream {
public:
    IpV4Stream(std::istream& ipv4_stream) :
     ipv4_stream_(ipv4_stream) {
    }

private:
    IpV4Stream& consume_ip_header() {
        // FIXME: We are hardcoding the size of the IP header.
        return read_bytes(20);
    }

    uint16_t read_unsigned_short() {
        uint16_t val;
        ipv4_stream_ >> val;
        return ntohs(val);
    }

    IpV4Stream& read_bytes(std::size_t num_bytes) {
        // FIXME: We are not guaranteed that a char is a byte in C++.
        char bytes[num_bytes];
        ipv4_stream_.get(bytes, num_bytes);
        return *this;
    }
    
    std::string to_str() {
        std::string s(std::istreambuf_iterator<char>(ipv4_stream_), {});
        return s;
    }

    std::istream& ipv4_stream_;
    friend EchoResponse;
};


class EchoResponse {

public:
    EchoResponse(IpV4Stream& ipv4_stream);
    ~EchoResponse();

private:
    EchoResponse(uint16_t identifier, uint16_t sequence_number, 
        const std::string data);

    const uint16_t identifier_;
    const uint16_t sequence_number_;
    const std::string data_;
};


}  // namespace pingfs

#endif