#ifndef _BLOCK_
#define _BLOCK_

#include "block_data/block_data.hpp"
#include "block_data.pb.h"

#include <memory>
#include <string>

namespace pingfs {

class Block {
 public:
    Block(const BlockProto& proto);
    Block(BlockId block_id, const BlockDataProto& data_proto);
    Block(BlockId block_id, std::shared_ptr<const BlockDataProto> data);
    ~Block();

    BlockId get_block_id() const;

    std::shared_ptr<const BlockDataProto> get_data() const;

    bool operator==(const Block &other) const;

    bool operator!=(const Block &other) const;

 private:
    const BlockId block_id_;
    const std::shared_ptr<const BlockDataProto> data_;
};

}  // namespace pingfs

#endif
