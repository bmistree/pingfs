#ifndef _ID_SUPPLIER_
#define _ID_SUPPLIER_

#include <pingfs/block/block.hpp>

namespace pingfs {

class IdSupplier {
 public:
    virtual BlockId next_id() = 0;
    virtual void free_id(BlockId block_id) = 0;
};


}  // namespace pingfs

#endif
