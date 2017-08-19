#include <pingfs/block/block_data/block_data.hpp>


namespace pingfs {

// FIXME: This method should probably get replaced by a utility
// that wraps protobuf iterators to turn them into standard stl
// iterators.
std::vector<BlockId> BlockData::iters_to_vector(
    const google::protobuf::RepeatedField<BlockId>& repeated_field) {
    std::vector<BlockId> vec;
    for (auto iter = repeated_field.begin(); iter != repeated_field.end();
         ++iter) {
        vec.push_back(*iter);
    }
    return vec;
}

}  // namespace pingfs
