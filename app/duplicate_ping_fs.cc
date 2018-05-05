#include <boost/exception/diagnostic_information.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/program_options.hpp>

#include <pingfs/fs/block_fuse.hpp>
#include <pingfs/fs/revertible_block_fuse.hpp>

#include <pingfs/fs/fuse_wrapper.hpp>

#include <pingfs/block/block_manager/ping/duplicate_ping_block_manager.hpp>
#include <pingfs/block/block_manager/ping/ping_block_manager.hpp>
#include <pingfs/block/block_manager/ping/pass_through_translator.hpp>

#include <pingfs/block/block_manager/id_supplier/counter_supplier.hpp>

#include <pingfs/block/block_manager/ping/track_freed_service.hpp>

#include <pingfs/util/log.hpp>

#include <memory>
#include <iostream>
#include <thread>

#include "args_helper.hpp"

const std::size_t NUM_IO_THREADS = 10;


bool parse_command_line(int argc, char** argv,
    std::string* hostname, std::string* mount_point,
    int64_t* delay_ms) {
    boost::program_options::options_description desc("Options");
    desc.add_options()
        ("help", "Print help messages")
        ("hostname",
            boost::program_options::value<std::string>(hostname)->required(),
            "Name of host to ping")
        ("mount_point",
            boost::program_options::value<std::string>(mount_point)->required(),
            "Path to directory to mount pingfs on")
        ("delay_ms",
            boost::program_options::value<int64_t>(delay_ms)->required(),
            "Number of ms to delay for next ms");

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


/**
 * Tests that running a ping-based file system that
 * issues duplicate pings to a single endpoint.
 */
int main(int argc, char** argv) {
    std::string hostname;
    std::string mount_point;
    int64_t delay_ms;

    if (!parse_command_line(
            argc, argv, &hostname, &mount_point, &delay_ms)) {
        return 1;
    }
    pingfs::Log::init_cout(pingfs::LogLevel::DEBUG);

    boost::asio::io_service io_service;

    std::shared_ptr<pingfs::DuplicatePingBlockManager> block_manager =
        std::make_shared<pingfs::DuplicatePingBlockManager>(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::milliseconds(delay_ms)),
            gen_counter_supplier(),
            gen_block_service(&io_service, hostname));

    std::shared_ptr<pingfs::BlockFuse> block_fuse =
        std::make_shared<pingfs::BlockFuse>(
            std::dynamic_pointer_cast<pingfs::PingBlockManager>(block_manager),
            FS_ID);
    block_fuse->set_global_wrapper();
    std::shared_ptr<struct fuse_operations> ops =
        block_fuse->generate();

    std::vector<char*> fuse_args;
    fuse_params(mount_point, false /* debug */, &fuse_args);

    // Allow multiple threads to service io events
    for (std::size_t i = 0; i < NUM_IO_THREADS; ++i) {
        std::thread t1(run_io_service, &io_service);
        t1.detach();
    }
    return fuse_main(fuse_args.size() - 1,
        fuse_args.data(), ops.get());
}
