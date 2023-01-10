cmake_minimum_required(VERSION 3.10)
project(CppTest)

set(CMAKE_CXX_STANDARD 14)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../../build/bin)
message(${PROJECT_SOURCE_DIR})
add_executable(host hello.cpp)
#add_executable(heuristicPartition GraphPartitionAlgo.cpp Block.cpp Block.h Partition.cpp Partition.h basic.h)