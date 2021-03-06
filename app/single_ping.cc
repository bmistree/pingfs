#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/program_options.hpp>
#include <pingfs/ping/echo_response.hpp>
#include <pingfs/ping/ping.hpp>
#include <pingfs/util/subscriber.hpp>

#include <iostream>
#include <chrono>
#include <thread>

class EchoRespSubscriber : public pingfs::Subscriber<pingfs::EchoResponse> {
 public:
    EchoRespSubscriber()
     : identifier_(nullptr) {
    }

    ~EchoRespSubscriber() {
        if (identifier_ == nullptr) {
            delete identifier_;
        }
    }

    void process(const pingfs::EchoResponse& notice) override {
        if (identifier_ == nullptr) {
            identifier_ = new uint16_t;
        }
        *identifier_ = notice.get_identifier();
    }

    const uint16_t* get_identifier() const {
        return identifier_;
    }

 private:
    uint16_t* identifier_;
};

/**
 * Returns true if command line parsing passed; false otherwise.
 */
bool parse_command_line(int argc, char** argv,
    std::string* hostname, std::string* ping_content) {
    boost::program_options::options_description desc("Options");
    desc.add_options()
        ("help", "Print help messages")
        ("hostname",
            boost::program_options::value<std::string>(hostname)->required(),
            "Name of host to ping")
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
    std::string hostname;
    std::string ping_content;

    if (!parse_command_line(argc, argv, &hostname, &ping_content)) {
        return 1;
    }
    boost::asio::io_service io_service;
    pingfs::Ping ping(&io_service);

    EchoRespSubscriber subscriber;
    ping.subscribe(&subscriber);

    // Actually issue a ping
    boost::asio::ip::icmp::endpoint endpoint = ping.resolve(hostname);
    uint16_t identifier = 3533;
    ping.ping(ping_content, endpoint, identifier, 1111);
    std::thread t1(run_io_service, &io_service);
    t1.detach();
    // Wait for a couple of seconds
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Print out received identifier
    const uint16_t* received_identifier = subscriber.get_identifier();
    if (received_identifier == nullptr) {
        std::cout << "\nNo response\n";
    } else if (*received_identifier == identifier) {
        std::cout << "\nResponse with correct identifer\n";
    } else {
        std::cout << "\nResponse with incorrect identifer\n";
    }

    return 0;
}
