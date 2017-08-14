#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/program_options.hpp>
#include <pingfs/ping/ping.hpp>

/**
 * Returns true if command line parsing passed; false otherwise.
 */
bool parse_command_line(int argc, char** argv, 
    std::string& hostname, std::string& ping_content) {
    boost::program_options::options_description desc("Options");
    desc.add_options()
        ("help", "Print help messages")
        ("hostname",
            boost::program_options::value<std::string>(&hostname)->required(),
            "Name of host to ping")
        ("content",
            boost::program_options::value<std::string>(&ping_content)->required(),
            "Content to put in ping body");

    boost::program_options::variables_map vm;
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc),
        vm);

    // The user specified the help flag
    if (vm.count("help")) {
        std::cout << desc << "\n";
        return false;
    }
    // Throw exceptions for missing arguments
    try {
        boost::program_options::notify(vm);
    } catch(std::exception ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return false;
    }
    return true;
}


int main(int argc, char** argv) {
    std::string hostname;
    std::string ping_content;

    if (!parse_command_line(argc, argv, hostname, ping_content)) {
        return 1;
    }
    boost::asio::io_service io_service;
    pingfs::Ping ping(io_service);
    boost::asio::ip::icmp::endpoint endpoint = ping.resolve(hostname);
    ping.ping(ping_content, endpoint, 3353, 1111);
    io_service.run();
    return 0;
}
