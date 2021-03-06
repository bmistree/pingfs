#ifndef _FS_UTIL_TEST_
#define _FS_UTIL_TEST_

#include <pingfs/fs/fs_util.hpp>

#include <string>
#include <vector>

#include "gtest/gtest.h"


TEST(FsUtil, SeparatePathRootDir) {
    ASSERT_EQ(pingfs::FsUtil::separate_path("/"),
       std::vector<std::string>({ "/" }));
}

TEST(FsUtil, SeparatePathSingleFile) {
    ASSERT_EQ(pingfs::FsUtil::separate_path("/a.txt"),
        std::vector<std::string>({ "/", "a.txt" }));
}

TEST(FsUtil, SeparatePathNestedDirectories) {
    ASSERT_EQ(pingfs::FsUtil::separate_path("/a/b/cdef/g.txt"),
        std::vector<std::string>({ "/", "a", "b", "cdef", "g.txt" }));
}

TEST(FsUtil, Join) {
    ASSERT_EQ(pingfs::FsUtil::join({"/", "a", "bcd"}), "/a/bcd");
}

TEST(FsUtil, JoinAndSeparate) {
    std::string test_path = "/a/b/cdef/g.txt";
    ASSERT_EQ(pingfs::FsUtil::join(pingfs::FsUtil::separate_path(test_path)),
        test_path);
}


#endif
