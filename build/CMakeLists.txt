cmake_minimum_required(VERSION 2.8)
project (pingfs)

set(LIB_FOLDER "../lib")
set(LIB_INC_FOLDER "${LIB_FOLDER}/include")
set(LIB_SRC_FOLDER "${LIB_FOLDER}/src")

list( APPEND CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS} -g -Wall")

include_directories(${LIB_INC_FOLDER})
file(GLOB LIB_PING_SRC_FILES "${LIB_SRC_FOLDER}/ping/*.cc")

add_library(
  pingfs_lib
  ${LIB_PING_SRC_FILES})


####### Add boost ########
find_package(Boost 1.58.0)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  endif() 
