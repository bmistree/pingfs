cmake_minimum_required(VERSION 2.8)
project (pingfs)

set(LIB_FOLDER "../lib")
set(LIB_INC_FOLDER "${LIB_FOLDER}/include")
set(LIB_SRC_FOLDER "${LIB_FOLDER}/src")

# Add flags for fuse
execute_process(COMMAND pkg-config fuse --cflags --libs OUTPUT_VARIABLE FUSE_FLAGS)
string(STRIP "${FUSE_FLAGS}" FUSE_FLAGS)
# FIXME: I should not have hard coded this
set(FUSE_LIBRARY "/usr/lib/x86_64-linux-gnu/libfuse.so")

add_definitions(-DFUSE_USE_VERSION=25)

# Set compile parameters
list( APPEND CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS} -g -Wall -pthread ${FUSE_FLAGS}")


######## Compile protocol buffers #########
file(GLOB PROTO_FILES  "${LIB_FOLDER}/../proto/*.proto")

INCLUDE(FindProtobuf)
FIND_PACKAGE(Protobuf REQUIRED)
INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})
PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HEADER ${PROTO_FILES})
ADD_LIBRARY(pingfs_proto ${PROTO_HEADER} ${PROTO_SRC})
include_directories(".")


######### Generate pingfs_lib #########
include_directories(${LIB_INC_FOLDER})
file(GLOB LIB_UTIL_SRC_FILES "${LIB_SRC_FOLDER}/util/*.cc")
file(GLOB LIB_PING_SRC_FILES "${LIB_SRC_FOLDER}/ping/*.cc")
file(GLOB LIB_BLOCK_SRC_FILES "${LIB_SRC_FOLDER}/block/*.cc")
file(GLOB LIB_BLOCK_DATA_SRC_FILES "${LIB_SRC_FOLDER}/block/block_data/*.cc")
file(GLOB LIB_BLOCK_MANAGER_SRC_FILES "${LIB_SRC_FOLDER}/block/block_manager/*.cc")
file(GLOB LIB_PING_BLOCK_MANAGER_SRC_FILES "${LIB_SRC_FOLDER}/block/block_manager/ping/*.cc")
file(GLOB LIB_ID_SUPPLIER_SRC_FILES "${LIB_SRC_FOLDER}/block/block_manager/id_supplier/*.cc")
file(GLOB LIB_FS_SRC_FILES "${LIB_SRC_FOLDER}/fs/*.cc")

add_library(
  pingfs_lib
  ${LIB_UTIL_SRC_FILES}
  ${LIB_PING_SRC_FILES}
  ${LIB_BLOCK_SRC_FILES}
  ${LIB_BLOCK_DATA_SRC_FILES}
  ${LIB_BLOCK_MANAGER_SRC_FILES}
  ${LIB_PING_BLOCK_MANAGER_SRC_FILES}
  ${LIB_ID_SUPPLIER_SRC_FILES}
  ${LIB_FS_SRC_FILES})


####### Add boost ########
find_package(Boost 1.58.0 REQUIRED COMPONENTS system program_options)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()


######## Include gtest ########
include(ExternalProject)

# Download and install GoogleTest
ExternalProject_Add(
  gtest
  URL https://github.com/google/googletest/archive/release-1.8.0.zip
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
  # Disable install step
  INSTALL_COMMAND ""
  )

# Create a libgtest target to be used as a dependency by test programs
add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)

# Set gtest properties
ExternalProject_Get_Property(gtest source_dir binary_dir)
set_target_properties(libgtest PROPERTIES
  "IMPORTED_LOCATION" "${binary_dir}/googlemock/gtest/libgtest.a"
  "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
  )
include_directories("${source_dir}/googletest/include")


##### app builds #######
set(APP_FOLDER "../app")

# Tests issuing a single ping and waiting on a response
add_executable(single_ping "${APP_FOLDER}/single_ping.cc")
target_link_libraries(single_ping pingfs_lib ${Boost_LIBRARIES} pingfs_proto ${PROTOBUF_LIBRARY} ${FUSE_LIBRARY})

# Tests building an in-memory fuse filesystem
add_executable(in_memory "${APP_FOLDER}/in_memory.cc")
target_link_libraries(in_memory pingfs_lib ${Boost_LIBRARIES} pingfs_proto ${PROTOBUF_LIBRARY} ${FUSE_LIBRARY})

# Tests building a file system that stores
# data in pings to a single host
add_executable(single_host_fs "${APP_FOLDER}/single_host_fs.cc")
target_link_libraries(single_host_fs pingfs_lib ${Boost_LIBRARIES} pingfs_proto ${PROTOBUF_LIBRARY} ${FUSE_LIBRARY})


##### test builds ######
set(TEST_FOLDER "../test")

add_executable(all_tests "${TEST_FOLDER}/test_main.cc")
target_link_libraries(all_tests pingfs_lib ${Boost_LIBRARIES} libgtest pingfs_proto ${PROTOBUF_LIBRARY} ${FUSE_LIBRARY})
