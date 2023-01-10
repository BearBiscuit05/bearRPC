cmake_minimum_required(VERSION 3.23)
project(CppTest)

set(CMAKE_CXX_STANDARD 14)

add_executable(heuristicPartition GraphPartitionAlgo.cpp Block.cpp Block.h Partition.cpp Partition.h basic.h)