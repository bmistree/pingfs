#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/program_options.hpp>

#include <pingfs/ping/spoof/ping_spoof.hpp>

#include <iostream>
#include <chrono>
#include <thread>

/**
 * Returns true if command line parsing passed; false otherwise.
 */
bool parse_command_line(int argc, char** argv,
    std::string* target_hostname,
    std::string* spoofed_hostname,
    std::string* ping_content) {
    boost::program_options::options_description desc("Options");
    desc.add_options()
        ("help", "Print help messages")
        ("target_hostname",
            boost::program_options::value<std::string>(target_hostname)->required(),
            "Name of host to ping")
        ("spoofed_hostname",
            boost::program_options::value<std::string>(spoofed_hostname)->required(),
            "Name of host to spoof source as")
        ("content",
            boost::program_options::value<std::string>(
                ping_content)->required(),
            "Content to put in ping body");

    boost::program_options::variables_map vm;
    // Throw exceptions for missing/incorrect arguments
    try {
        boost::program_options::store(
            boost::program_options::parse_command_line(argc, argv, desc),
            vm);

        // The user specified the help flag
        if (vm.count("help")) {
            std::cout << desc << "\n";
            return false;
        }
        boost::program_options::notify(vm);
    } catch(std::exception& ex) {
        std::cerr << "\nError: " << ex.what() << "\n";
        std::cerr << "\n" << desc << "\n";
        return false;
    }
    return true;
}

void run_io_service(boost::asio::io_service* io_service) {
    io_service->run();
}

int main(int argc, char** argv) {
    std::string target_hostname;
    std::string spoofed_hostname;
    std::string ping_content;

    if (!parse_command_line(argc, argv, &target_hostname,
            &spoofed_hostname, &ping_content)) {
        return 1;
    }
    boost::asio::io_service io_service;
    pingfs::PingSpoof ping(&io_service);

    boost::asio::ip::address_v4 target = 
        boost::asio::ip::address_v4::from_string(target_hostname);
    boost::asio::ip::address_v4 spoofed_src = 
        boost::asio::ip::address_v4::from_string(spoofed_hostname);

    ping.send(
        ping_content,
        3533 /* identifier */,
        1111 /* sequence_number */,
        spoofed_src,
        target);

    std::thread t1(run_io_service, &io_service);
    t1.detach();
    // Wait for a couple of seconds
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 0;
}
