#include "gtest/gtest.h"

#include "lib/block/block_test.hpp"
#include "lib/block/memory_block_manager_test.hpp"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

