cmake_minimum_required(VERSION 2.8)
project (pingfs)

set(LIB_FOLDER "../lib")
set(LIB_INC_FOLDER "${LIB_FOLDER}/include")
set(LIB_SRC_FOLDER "${LIB_FOLDER}/src")

list( APPEND CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS} -g -Wall -pthread")

include_directories(${LIB_INC_FOLDER})
file(GLOB LIB_PING_SRC_FILES "${LIB_SRC_FOLDER}/ping/*.cc")
file(GLOB LIB_BLOCK_SRC_FILES "${LIB_SRC_FOLDER}/block/*.cc")

add_library(
  pingfs_lib
  ${LIB_PING_SRC_FILES}
  ${LIB_BLOCK_SRC_FILES})


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
add_executable(single_ping "${APP_FOLDER}/single_ping.cc")
target_link_libraries(single_ping pingfs_lib ${Boost_LIBRARIES})

##### test builds ######
set(TEST_FOLDER "../test")

add_executable(all_tests "${TEST_FOLDER}/test_main.cc")
target_link_libraries(all_tests pingfs_lib ${Boost_LIBRARIES} libgtest)
