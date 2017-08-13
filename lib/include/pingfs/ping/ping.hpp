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
    std::string ping(const std::string& content, 
        const std::string& destination);

private:
    boost::asio::io_service& io_service_;
};

}  // namespace pingfs


#endif
