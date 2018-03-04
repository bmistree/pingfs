#include <boost/exception/diagnostic_information.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/program_options.hpp>

#include <pingfs/fs/block_fuse.hpp>

#include <pingfs/fs/fuse_wrapper.hpp>

#include <pingfs/block/block_manager/ping/ping_block_manager.hpp>
#include <pingfs/block/block_manager/ping/pass_through_translator.hpp>

#include <pingfs/block/block_manager/id_supplier/counter_supplier.hpp>

#include <pingfs/block/block_manager/ping/track_freed_service.hpp>

#include <memory>
#include <iostream>
#include <thread>

const uint16_t FS_ID = 55;

void copy_char(char** dest, const char* src,
    std::size_t src_len) {
    *dest = new char[src_len];
    strncpy(*dest, src, src_len + 1);
}

void fuse_params(const std::string& mount_point,
    bool debug, std::vector<char*>* fuse_args) {
    char* bin_name;
    copy_char(&bin_name, "dummy", 5);
    fuse_args->push_back(bin_name);
    if (debug) {
        char* debug_opt;
        copy_char(&debug_opt, "-d", 2);
        fuse_args->push_back(debug_opt);
    }

    char* mount_point_ptr;
    copy_char(
        &mount_point_ptr,
        mount_point.c_str(),
        mount_point.size());
    fuse_args->push_back(mount_point_ptr);
    fuse_args->push_back(nullptr);
}

bool parse_command_line(int argc, char** argv,
    std::string* hostname, std::string* mount_point,
    bool* debug) {
    boost::program_options::options_description desc("Options");
    desc.add_options()
        ("help", "Print help messages")
        ("hostname",
            boost::program_options::value<std::string>(hostname)->required(),
            "Name of host to ping")
        ("debug",
            boost::program_options::value<bool>(
                debug)->default_value(false),
            "Whether to print debug values")
        ("mount_point",
            boost::program_options::value<std::string>(mount_point)->required(),
            "Path to directory to mount pingfs on");

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


std::shared_ptr<pingfs::CounterSupplier> gen_counter_supplier() {
    return std::make_shared<pingfs::CounterSupplier>();
}


/**
 * @param remote_endpt A hostname to bounce messages
 * off of.
 */
std::shared_ptr<pingfs::PingBlockService> gen_block_service(
    boost::asio::io_service* io_service,
    const std::string remote_endpt) {

    std::shared_ptr<pingfs::Ping> ping =
        std::make_shared<pingfs::Ping>(io_service);

    std::shared_ptr<pingfs::BlockPingTranslator> translator =
        std::make_shared<pingfs::PassThroughTranslator>();

    return std::make_shared<pingfs::TrackFreedService>(
        FS_ID,
        ping,
        translator,
        remote_endpt);
}

void run_io_service(boost::asio::io_service* io_service) {
    io_service->run();
}

/**
 * Tests that running a ping-based file system that
 * issues pings to a single endpoint.
 */
int main(int argc, char** argv) {
    std::string hostname;
    std::string mount_point;
    bool debug;

    if (!parse_command_line(
            argc, argv, &hostname, &mount_point, &debug)) {
        return 1;
    }

    boost::asio::io_service io_service;

    std::shared_ptr<pingfs::PingBlockManager> block_manager =
        std::make_shared<pingfs::PingBlockManager>(
            gen_counter_supplier(),
            gen_block_service(&io_service, hostname));

    pingfs::BlockFuse block_fuse(block_manager, FS_ID);
    block_fuse.set_global_wrapper();
    std::shared_ptr<struct fuse_operations> ops =
        block_fuse.generate();

    std::vector<char*> fuse_args;
    fuse_params(mount_point, debug, &fuse_args);

    std::thread t1(run_io_service, &io_service);
    t1.detach();

    return fuse_main(fuse_args.size() - 1,
        fuse_args.data(), ops.get());
}
