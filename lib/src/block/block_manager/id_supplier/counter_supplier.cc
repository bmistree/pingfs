#include <pingfs/block/block_manager/id_supplier/counter_supplier.hpp>


namespace pingfs {

CounterSupplier::CounterSupplier()
  : next_id_(1) {
}

CounterSupplier::~CounterSupplier() {
}

BlockId CounterSupplier::next_id() {
    boost::mutex::scoped_lock locker(next_id_mutex_);
    if (next_id_ == 0) {
        throw "Exhausted id space";
    }

    return next_id_++;
}

void CounterSupplier::free_id(BlockId block_id) {
    // Do nothing for now; we assume that the id
    // space is large enough that we won't exhaust it.
}

}  // namespace pingfs
