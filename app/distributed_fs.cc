#include <boost/exception/diagnostic_information.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/program_options.hpp>

#include <pingfs/fs/distributed_block_fuse.hpp>

#include <pingfs/fs/fuse_wrapper.hpp>

#include <pingfs/block/block_manager/ping/ping_block_manager.hpp>
#include <pingfs/block/block_manager/ping/pass_through_translator.hpp>
#include <pingfs/block/block_manager/id_supplier/updating_id_supplier.hpp>

#include <pingfs/ping/spoof/ping_spoof.hpp>

#include <pingfs/util/log.hpp>

#include <memory>
#include <iostream>
#include <thread>

#include "args_helper.hpp"

const std::size_t NUM_IO_THREADS = 10;


bool parse_command_line(int argc, char** argv,
    std::string* bounce_ip, std::string* other_node_ip,
    std::string* mount_point, uint8_t* node_id) {
    boost::program_options::options_description desc("Options");
    desc.add_options()
        ("help", "Print help messages")
        ("bounce_ip",
            boost::program_options::value<std::string>(bounce_ip)->required(),
            "IP addr of machine to bounce pings off of")
        ("other_node_ip",
            boost::program_options::value<std::string>(other_node_ip)->required(),
            "IP address of other host in distributed fs")
        ("mount_point",
            boost::program_options::value<std::string>(mount_point)->required(),
            "Path to directory to mount pingfs on")
        ("node_id",
            boost::program_options::value<uint8_t>(node_id)->required(),
            "A unique numeric id among all fs nodes [0, 255]");

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

std::shared_ptr<pingfs::DistributedFreedService> gen_distributed_block_service(
    boost::asio::io_service* io_service,
    const std::string& bounce_ip,
    const std::string& other_node_ip) {

    std::shared_ptr<pingfs::Ping> ping =
        std::make_shared<pingfs::Ping>(io_service);

    std::shared_ptr<pingfs::BlockPingTranslator> translator =
        std::make_shared<pingfs::PassThroughTranslator>();

    std::shared_ptr<pingfs::SpoofInfo> spoof_info =
        std::make_shared<pingfs::SpoofInfo>(
            boost::asio::ip::address_v4::from_string(other_node_ip),
            boost::asio::ip::address_v4::from_string(bounce_ip));

    std::vector<std::shared_ptr<pingfs::SpoofInfo>> other_nodes;
    other_nodes.push_back(spoof_info);

    return std::make_shared<pingfs::DistributedFreedService>(
        FS_ID,
        ping,
        translator,
        bounce_ip,
        std::make_shared<pingfs::PingSpoof>(io_service),
        other_nodes);
}

std::shared_ptr<pingfs::UpdatingIdSupplier> gen_distributed_supplier(
    uint8_t low_order_bits) {
    return std::make_shared<pingfs::UpdatingIdSupplier>(low_order_bits);
}

/**
 * Tests that running a ping-based file system that
 * issues pings to a single endpoint.
 */
int main(int argc, char** argv) {
    std::string bounce_ip;
    std::string other_node_ip;
    std::string mount_point;
    uint8_t node_id;

    if (!parse_command_line(
            argc, argv, &bounce_ip, &other_node_ip, &mount_point, &node_id)) {
        return 1;
    }
    pingfs::Log::init_cout(pingfs::LogLevel::DEBUG);
    boost::asio::io_service io_service;

    std::shared_ptr<pingfs::UpdatingIdSupplier> updating_id_supplier =
        gen_distributed_supplier(node_id);
    std::shared_ptr<pingfs::DistributedFreedService> distributed_block_service =
        gen_distributed_block_service(&io_service, bounce_ip, other_node_ip);

    std::shared_ptr<pingfs::PingBlockManager> block_manager =
        std::make_shared<pingfs::PingBlockManager>(
            std::dynamic_pointer_cast<pingfs::IdSupplier>(updating_id_supplier),
            std::dynamic_pointer_cast<pingfs::PingBlockService>(distributed_block_service));

    std::shared_ptr<pingfs::DistributedBlockFuse> block_fuse =
        std::make_shared<pingfs::DistributedBlockFuse>(
            std::dynamic_pointer_cast<pingfs::BlockManager>(block_manager),
            FS_ID,
            updating_id_supplier,
            distributed_block_service);

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
