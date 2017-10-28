#ifndef _IP_V4_STREAM_
#define _IP_V4_STREAM_

#include <boost/asio.hpp>

#include <iostream>
#include <string>

namespace pingfs {

class EchoResponse;
class IpV4;

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

    uint8_t read_byte() {
        uint8_t val;
        ipv4_stream_->read(reinterpret_cast<char*>(&val), 1);
        return val;
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
    friend IpV4;
};

}  // namespace pingfs

#endif
