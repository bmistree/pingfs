#ifndef _PING_SPOOF_
#define _PING_SPOOF_

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/system/error_code.hpp>
#include <boost/thread/mutex.hpp>

#include <pingfs/ping/echo_request.hpp>

#include <memory>

namespace pingfs {

class PingSpoof {
 public:
    explicit PingSpoof(boost::asio::io_service* io_service);
    ~PingSpoof();

    void send(const std::string& content,
        uint16_t identifier,
        uint16_t sequence_number,
        const boost::asio::ip::address_v4& spoofed_src,
        const boost::asio::ip::address_v4& spoofed_target);

    void send(std::shared_ptr<const EchoRequest> echo_request,
        const boost::asio::ip::address_v4& spoofed_src,
        const boost::asio::ip::address_v4& spoofed_target);

 private:
    boost::asio::io_service* io_service_;
    boost::asio::ip::icmp::resolver resolver_;
    boost::asio::ip::icmp::socket sock_;
};

}  // namespace pingfs

#endif
