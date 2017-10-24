#ifndef _PASS_THROUGH_TRANSLATOR_TEST_
#define _PASS_THROUGH_TRANSLATOR_TEST_

#include <pingfs/block/block_manager/ping/pass_through_translator.hpp>

#include <memory>

static pingfs::EchoResponse request_to_response(
    std::shared_ptr<const pingfs::EchoRequest> request) {
    return pingfs::EchoResponse(
        request->get_identifier(),
        request->get_sequence_number(),
        request->get_body());
}

TEST(PassThroughTranslator, FromponseToRequest) {
    std::shared_ptr<const pingfs::Block> block =
        std::make_shared<const pingfs::Block>(
            1u,
            std::make_shared<const pingfs::FileContentsBlockData>(
                "some data"));

    // Serialize to echo request
    pingfs::PassThroughTranslator translator;
    uint16_t identifier = 35;
    uint16_t sequence_number = 88;

    std::shared_ptr<const pingfs::EchoRequest> request =
        translator.to_request(
            block, identifier, sequence_number);

    ASSERT_EQ(request->get_identifier(), identifier);
    ASSERT_EQ(request->get_sequence_number(),
        sequence_number);

    pingfs::EchoResponse response =
        request_to_response(request);

    // Test response deserializes into initially-serialized
    // block
    ASSERT_EQ(
        *translator.from_response(response),
        *block);
}


#endif
