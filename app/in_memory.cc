#include <pingfs/block/block_manager/memory_block_manager.hpp>
#include <pingfs/fs/block_fuse.hpp>

#include <pingfs/fs/fuse_wrapper.hpp>
#include <pingfs/block/block_manager/id_supplier/counter_supplier.hpp>

#include <pingfs/util/log.hpp>

#include <memory>


/**
 * Tests building an in-memory file system.
 */
int main(int argc, char** argv) {
    pingfs::Log::init_cout(pingfs::LogLevel::ERROR);
    
    std::shared_ptr<pingfs::MemoryBlockManager> block_manager =
        std::make_shared<pingfs::MemoryBlockManager>(
            std::make_shared<pingfs::CounterSupplier>());

    pingfs::BlockFuse block_fuse(block_manager, 55);
    block_fuse.set_global_wrapper();
    std::shared_ptr<struct fuse_operations> ops = block_fuse.generate();
    return fuse_main(argc, argv, ops.get());
}
