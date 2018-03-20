#ifndef _PING_
#define _PING_

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/system/error_code.hpp>
#include <boost/thread/mutex.hpp>

#include <pingfs/util/publisher.hpp>
#include <pingfs/util/subscriber.hpp>

#include <string>

#include "data_buffer.hpp"
#include "echo_request.hpp"
#include "echo_response.hpp"

namespace pingfs {

class Ping : public Publisher<EchoResponse> {
 public:
    explicit Ping(boost::asio::io_service* io_service);
    ~Ping();
    void ping(const std::string& content,
        const boost::asio::ip::icmp::endpoint& enpoint,
        uint16_t identifier, uint16_t sequence_number);

    void ping(const EchoRequest& request,
        const boost::asio::ip::icmp::endpoint& endpoint);

    boost::asio::ip::icmp::endpoint resolve(const std::string& destination);

 private:
    void handle_receive(const boost::system::error_code& code,
        std::size_t length);
    void set_handler();

    /**
     * @return Whether or not a full packet was consumed
     */
    bool check_notify();

    void internal_check_notify();

 private:
    boost::asio::io_service* io_service_;
    boost::asio::ip::icmp::resolver resolver_;
    boost::asio::ip::icmp::socket sock_;
    /**
     * Where we receive ping bytes. Note that we should
     * only set a single handler writing to this at a time
     * to avoid data corruption.
     */
    std::string reply_buffer_;

    DataBuffer data_buffer_;
    boost::mutex data_buffer_read_mutex_;
};

}  // namespace pingfs


#endif
