#ifndef _PING_
#define _PING_

#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>


namespace pingfs {

std::string ping(const std::string& content, const std::string& destination,
    boost::asio::io_service& io_service);

}  // namespace pingfs


#endif
