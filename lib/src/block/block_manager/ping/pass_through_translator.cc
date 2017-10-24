#include <block_data.pb.h>

#include <pingfs/block/block_manager/ping/pass_through_translator.hpp>

namespace pingfs {

PassThroughTranslator::PassThroughTranslator() {
}

PassThroughTranslator::~PassThroughTranslator() {
}

std::shared_ptr<const Block> PassThroughTranslator::from_response(
    const EchoResponse& response) {
    proto::BlockProto proto;
    proto.ParseFromString(response.get_data());
    return std::make_shared<const Block>(proto);
}

std::shared_ptr<const EchoRequest> PassThroughTranslator::to_request(
    std::shared_ptr<const Block> block,
    uint16_t identifier, uint16_t sequence_number) {

    proto::BlockProto proto;
    block->gen_proto(&proto);

    std::string serialized;
    proto.SerializeToString(&serialized);

    return std::make_shared<const EchoRequest>(
        identifier, sequence_number,
        serialized);
}

}  // namespace pingfs
