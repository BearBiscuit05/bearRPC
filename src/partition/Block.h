//
// Created by 74203 on 2022/12/26.
//

#ifndef CPPTEST_BLOCK_H
#define CPPTEST_BLOCK_H
#include "basic.h"

class Block{
public:
    int srcNodeID;
    int blockId;
    int partitionId;
    std::vector<std::vector<int>> NodeList;
    int BlockSize;
    int trainNodeNumber;
    int valNodeNumber;
    int testNodeNumber;
};


#endif //CPPTEST_BLOCK_H
