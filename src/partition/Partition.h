//
// Created by 74203 on 2022/12/26.
//

#ifndef CPPTEST_PARTITION_H
#define CPPTEST_PARTITION_H
#include "basic.h"
#include "Block.h"

class Partition{
public:
    int partitionId;
    std::vector<int> NodeList;
    int size;
    int trainSize;
    int valSize;
    int testSize;
    int maxTrainSize;
    int maxValSize;
    int maxTestSize;

    void mergeBlock(Block& block);
};


#endif //CPPTEST_PARTITION_H


