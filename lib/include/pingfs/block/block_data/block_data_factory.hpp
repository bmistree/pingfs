#ifndef _BLOCK_DATA_FACTORY_
#define _BLOCK_DATA_FACTORY_

#include "block_data.pb.h"
#include "block_data.hpp"
#include <memory>


namespace pingfs {

class BlockDataFactory {

public:
    static std::shared_ptr<const BlockData> generate(
        const BlockDataProto& proto);

private:
    BlockDataFactory() {
    }

};

}  // namespace pingfs

#endif
