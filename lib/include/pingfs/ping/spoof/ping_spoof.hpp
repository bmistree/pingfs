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

    void send(std::shared_ptr<EchoRequest> request,
        const boost::asio::ip::address_v4& spoofed_src,
        const boost::asio::ip::icmp::endpoint& target);

 private:
    boost::asio::io_service* io_service_;
    boost::asio::ip::icmp::resolver resolver_;
    boost::asio::ip::icmp::socket sock_;
};

}  // namespace pingfs

#endif
