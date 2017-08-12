#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <pingfs/ping/ping.hpp>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr<<"\nUsage: ./single_ping [HOST] [BODY CONTENT]\n\n";
        return 1;
    }
    std::string hostname(argv[1]);
    std::string ping_content(argv[2]);

    boost::asio::io_service io_service;
    pingfs::ping(ping_content, hostname, io_service);
    io_service.run();
    return 0;
}
