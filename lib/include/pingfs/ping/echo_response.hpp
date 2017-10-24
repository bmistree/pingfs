#ifndef _ECHO_RESPONSE_
#define _ECHO_RESPONSE_

#include <boost/asio.hpp>

#include <iostream>
#include <string>

namespace pingfs {

class EchoResponse;

class IpV4Stream {
 public:
    explicit IpV4Stream(std::istream* ipv4_stream) :
     ipv4_stream_(ipv4_stream) {
    }

 private:
    IpV4Stream& consume_ip_header() {
        // FIXME: We are hardcoding the size of the IP header.
        return read_bytes(20);
    }

    uint16_t read_unsigned_short() {
        uint16_t val;
        ipv4_stream_->read(reinterpret_cast<char*>(&val), 2);
        return ntohs(val);
    }

    IpV4Stream& read_bytes(std::size_t num_bytes) {
        // FIXME: We are not guaranteed that a char is a byte in C++.
        // FIXME 2: It's probably inefficient to hit the memory
        // allocator here, but this fixes a cpplint error, which
        // warns against putting potentially unbounded space on the
        // stack.
        char* bytes = new char[num_bytes];
        ipv4_stream_->read(bytes, num_bytes);
        delete bytes;
        return *this;
    }

    std::string to_str() {
        std::string s(std::istreambuf_iterator<char>(*ipv4_stream_), {});
        return s;
    }

    std::istream* ipv4_stream_;
    friend EchoResponse;
};


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
