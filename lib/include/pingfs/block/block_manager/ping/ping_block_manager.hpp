#ifndef _PING_BLOCK_MANAGER_
#define _PING_BLOCK_MANAGER_

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <pingfs/block/block_manager/async_block_manager.hpp>
#include <pingfs/block/block_request.hpp>
#include <pingfs/block/block_response.hpp>
#include <pingfs/block/block_data/block_data.hpp>
#include <pingfs/ping/ping.hpp>

#include <memory>
#include <string>

#include "ping_block_service.hpp"

namespace pingfs {

class PingBlockManager : public AsyncBlockManager {
 private:
    using BlockPtr = std::shared_ptr<const Block>;
    using DataPtr = std::shared_ptr<const BlockData>;
    using ResponsePtr = std::shared_ptr<const BlockResponse>;

 public:
    PingBlockManager(
        std::shared_ptr<IdSupplier> id_supplier,
        std::shared_ptr<PingBlockService> ping_block_service);

    virtual ~PingBlockManager();

    BlockPtr create_block(DataPtr data) override;
    void free_block(BlockId block_id) override;

 protected:
    std::shared_ptr<PingBlockService> ping_block_service_;
};

}  // namespace pingfs

#endif
