#ifndef _COUNTER_SUPPLIER_
#define _COUNTER_SUPPLIER_

#include <boost/thread/mutex.hpp>

#include <pingfs/block/block.hpp>
#include "id_supplier.hpp"

namespace pingfs {

class CounterSupplier : public IdSupplier {
 public:
    CounterSupplier();
    virtual ~CounterSupplier();
    BlockId next_id() override;
    void free_id(BlockId block_id) override;
 private:
    BlockId next_id_;
    boost::mutex next_id_mutex_;
};

}  // namespace pingfs

#endif
