#include <pingfs/block/block_manager/id_supplier/updating_id_supplier.hpp>


namespace pingfs {

UpdatingIdSupplier::UpdatingIdSupplier(uint8_t fixed_lower_order_bits)
 : fixed_low_order_bits_(fixed_lower_order_bits),
   high_order_bits_(1) {
}

UpdatingIdSupplier::~UpdatingIdSupplier() {
}

BlockId UpdatingIdSupplier::next_id() {
    boost::mutex::scoped_lock locker(next_id_mutex_);
    BlockId new_id = (high_order_bits_ << 8) + fixed_low_order_bits_;
    ++high_order_bits_;
    return new_id;
}

void UpdatingIdSupplier::update_if_less_than(BlockId block_id) {
    boost::mutex::scoped_lock locker(next_id_mutex_);

    BlockId just_high_order = block_id >> 8;
    if (just_high_order > high_order_bits_) {
        high_order_bits_ = just_high_order + 1;
    }
}

void UpdatingIdSupplier::free_id(BlockId block_id) {
    // Do nothing for now; we assume that the id
    // space is large enough that we won't exhaust it.
}

}  // namespace pingfs
