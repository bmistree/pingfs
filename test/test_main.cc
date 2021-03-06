#include "gtest/gtest.h"

#include "lib/block/block_test.hpp"
#include "lib/block/block_manager/memory_block_manager_test.hpp"
#include "lib/block/block_data/block_data_factory_test.hpp"
#include "lib/block/block_data/dir_file_block_data_test.hpp"
#include "lib/block/block_data/file_contents_block_data_test.hpp"
#include "lib/block/block_data/link_block_data_test.hpp"
#include "lib/block/block_data/read_write_execute_test.hpp"
#include "lib/block/block_data/file_type_test.hpp"
#include "lib/block/block_data/mode_test.hpp"
#include "lib/block/block_data/stat_test.hpp"
#include "lib/block/block_manager/async_response_test.hpp"
#include "lib/block/block_manager/async_block_manager_test.hpp"
#include "lib/block/block_manager/ping/pass_through_translator_test.hpp"

#include "lib/fs/fs_util_test.hpp"
#include "lib/fs/block_fuse_test.hpp"
#include "lib/ping/echo_request_test.hpp"
#include "lib/util/rate_queue_test.hpp"


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
