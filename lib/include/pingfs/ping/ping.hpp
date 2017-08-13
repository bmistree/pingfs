#ifndef _PING_
#define _PING_

#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace pingfs {

class Ping {
public:
    Ping(boost::asio::io_service& io_service);
    ~Ping();
    void ping(const std::string& content, 
        const std::string& destination, uint16_t identifier,
        uint16_t sequence_number);

private:
    boost::asio::io_service& io_service_;
    boost::asio::ip::icmp::resolver resolver_;
    boost::asio::ip::icmp::socket sock_;
};

}  // namespace pingfs


#endif
