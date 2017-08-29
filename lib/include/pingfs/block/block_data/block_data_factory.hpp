#ifndef _BLOCK_DATA_FACTORY_
#define _BLOCK_DATA_FACTORY_

#include <block_data.pb.h>
#include <memory>
#include "block_data.hpp"

namespace pingfs {

class BlockDataFactory {
 public:
    static std::shared_ptr<const BlockData> generate(
        const proto::BlockDataProto& proto);

 private:
    BlockDataFactory() {
    }
};

}  // namespace pingfs

#endif
