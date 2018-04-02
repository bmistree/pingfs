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
    EchoRespSubscriber(pingfs::Ping* ping,
        boost::asio::ip::icmp::endpoint endpoint)
     : ping_(ping),
       endpoint_(endpoint),
       num_pings_(0) {
    }

    ~EchoRespSubscriber() {
    }

    void issue_ping() {
        ping_->ping("some content" /* ping_content */,
            endpoint_, 3533 /* identifier */,
            1111 /* sequence_number */);
    }

    void process(const pingfs::EchoResponse& notice) override {
        ++num_pings_;
        std::thread t1(std::bind(&EchoRespSubscriber::issue_ping, this));
        t1.detach();
    }

    int get_num_pings() const {
        return num_pings_;
    }

 private:
    pingfs::Ping* ping_;
    boost::asio::ip::icmp::endpoint endpoint_;
    uint16_t num_pings_;
};

/**
 * Returns true if command line parsing passed; false otherwise.
 */
bool parse_command_line(int argc, char** argv,
    std::string* hostname, uint16_t* num_pings, std::string* log_dir) {
    boost::program_options::options_description desc("Options");
    desc.add_options()
        ("help", "Print help messages")
        ("hostname",
            boost::program_options::value<std::string>(hostname)->required(),
            "Name of host to ping")
        ("log-dir",
            boost::program_options::value<std::string>(
                log_dir)->required(),
            "Directory to log to")
        ("num-pings",
            boost::program_options::value<uint16_t>(
                num_pings)->required(),
            "Num pings");

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
    uint16_t num_pings;
    std::string log_dir;

    if (!parse_command_line(argc, argv, &hostname, &num_pings, &log_dir)) {
        return 1;
    }

    boost::asio::io_service io_service;
    pingfs::Ping ping(&io_service);
    boost::asio::ip::icmp::endpoint endpoint = ping.resolve(hostname);

    EchoRespSubscriber subscriber(&ping, endpoint);
    ping.subscribe(&subscriber);

    // Actually issue a ping
    std::thread t1(run_io_service, &io_service);
    t1.detach();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    subscriber.issue_ping();

    while(subscriber.get_num_pings() < num_pings) {
        // Wait for a couple of seconds
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    return 0;
}
