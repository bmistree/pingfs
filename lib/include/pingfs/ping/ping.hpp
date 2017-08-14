#ifndef _PING_
#define _PING_

#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/system/error_code.hpp>


namespace pingfs {

class Ping {
public:
    Ping(boost::asio::io_service& io_service);
    ~Ping();
    void ping(const std::string& content, 
        const boost::asio::ip::icmp::endpoint& enpoint,
        uint16_t identifier, uint16_t sequence_number);

    boost::asio::ip::icmp::endpoint resolve(const std::string& destination);

private:
    void handle_receive(const boost::system::error_code& code,
        std::size_t length);

private:
    boost::asio::io_service& io_service_;
    boost::asio::ip::icmp::resolver resolver_;
    boost::asio::ip::icmp::socket sock_;
    boost::asio::streambuf reply_buffer_;
};

}  // namespace pingfs


#endif
