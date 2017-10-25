#ifndef _ECHO_REQUEST_TEST_
#define _ECHO_REQUEST_TEST_

#include <pingfs/ping/echo_request.hpp>
#include <pingfs/ping/echo_response.hpp>

#include <string>

TEST(EchoRequest, ResponseConstructor) {
    pingfs::EchoRequest direct_request(
        55 /* identifier */,
        93 /* sequence_number */,
        "some body" /* body */);
    pingfs::EchoRequest request_from_response(
        pingfs::EchoResponse(55 /* identifier */,
            93 /* sequence_number */,
            "some body" /* body */));

    ASSERT_EQ(request_from_response, direct_request);
}

#endif
