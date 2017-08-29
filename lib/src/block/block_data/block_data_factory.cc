#include <pingfs/block/block_data/block_data.hpp>
#include <pingfs/block/block_data/block_data_factory.hpp>

#include <pingfs/block/block_data/dir_block_data.hpp>
#include <pingfs/block/block_data/file_contents_block_data.hpp>
#include <pingfs/block/block_data/file_start_block_data.hpp>
#include <pingfs/block/block_data/link_block_data.hpp>

namespace pingfs {

std::shared_ptr<const BlockData> BlockDataFactory::generate(
    const proto::BlockDataProto& proto) {

    if (proto.has_dir()) {
        return std::make_shared<const DirBlockData>(proto.dir());
    }

    if (proto.has_file_contents()) {
        return std::make_shared<const FileContentsBlockData>(
            proto.file_contents());
    }

    if (proto.has_file_start()) {
        return std::make_shared<const FileStartBlockData>(
            proto.file_start());
    }

    if (proto.has_link()) {
        return std::make_shared<const LinkBlockData>(
            proto.link());
    }

    throw "Unknown protocol buffer to deserialize";
}

}  // namespace pingfs
