#ifndef _UPDATING_ID_SUPPLIER_
#define _UPDATING_ID_SUPPLIER_

#include <boost/thread/mutex.hpp>

#include <pingfs/block/block.hpp>
#include "id_supplier.hpp"


namespace pingfs {

class UpdatingIdSupplier : public IdSupplier {
 public:
    UpdatingIdSupplier(uint8_t fixed_low_order_bits);
    virtual ~UpdatingIdSupplier();
    virtual BlockId next_id() override;
    virtual void free_id(BlockId block_id) override;

    /**
     * If the high order bits of {@code block_id} are greater than
     * the high order bits that we're tracking, then update local
     * high order bits.
     */
    void update_if_less_than(BlockId block_id);

private:
    // We will reserve the bottom 8 bits of a block id
    // for an unchanging value, and the top 56 bits
    // for a fixed value.
    uint8_t fixed_low_order_bits_;
    uint64_t high_order_bits_;

    boost::mutex next_id_mutex_;
};


}  // namespace pingfs

#endif
