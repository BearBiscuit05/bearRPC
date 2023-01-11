#include <iostream>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <cassert>
#include <string>
#include <algorithm>

using namespace std;

class Block{
public:
    int srcNodeID;
    int blockId;
    int partitionId;
    std::vector<std::vector<int>> NodeList;
    std::unordered_map<int,int> nodeViewed;
    std::vector<int> holoNode;
    int BlockSize = 0;
    int holoLayer = 3;
    int trainNodeNumber=0;
    int valNodeNumber=0;
    int testNodeNumber=0;

    // 对方块的信息值进行赋值
    void refactorBlock(int ID){
        blockId = ID;
        partitionId = -1;
        BlockSize = 0;
        for(auto& nodes : NodeList){
            BlockSize += nodes.size();
        }
    }
};

class Partition{
public:
    int partitionId=-1;
    std::unordered_map<int,bool> NodeViewed;
    std::vector<int> NodeList={};
    int size=0;
    int trainSize=0;
    int valSize=0;
    int testSize=0;
    int maxTrainSize{};
    int maxValSize{};
    int maxTestSize{};

    Partition(){}

    Partition(int ID, int maxTrain, int maxVal, int maxTest){
        partitionId = ID;
        maxTrainSize = maxTrain;
        maxValSize = maxVal;
        maxTestSize = maxTest;
    }

    void mergeBlock(Block& block){
        trainSize += block.trainNodeNumber;
        valSize += block.valNodeNumber;
        testSize += block.testNodeNumber;
        for(auto& nodes : block.NodeList){
            size += nodes.size();
            for(auto& node : nodes){
                if(NodeViewed.find(node) == NodeViewed.end()){
                    NodeViewed[node] = true;
                    NodeList.push_back(node);
                }
            }
        }
    }

    void compareWithBlock(Block& block)
    {
        // 通过比较交叉项来检测是否合适该分区
    }
};

// test success
void fileToGraph(string filepath, unordered_map<int,vector<int>>& node_map)
{
    std::ifstream Gin(filepath+".txt");
    assert(Gin.is_open());
    while(!Gin.eof())
    {
        int src, dst;
        Gin >> src >> dst;
        node_map[src].push_back(dst);
        node_map[dst].push_back(src);
    }
    Gin.close();
}

// 读取训练集
void readNodeClass(string filepath, vector<vector<int>>& NodeClassList)
{
    string trainPath = filepath+"_train.txt";
    string valPath = filepath+"_val.txt";
    string testPath = filepath+"_test.txt";
    std::ifstream Gin_train(trainPath);
    assert(Gin_train.is_open());
    while(!Gin_train.eof())
    {
        int nodeId;
        Gin_train >> nodeId;
        NodeClassList[0].emplace_back(nodeId);
    }
    Gin_train.close();

    std::ifstream Gin_val(valPath);
    assert(Gin_val.is_open());
    while(!Gin_val.eof())
    {
        int nodeId;
        Gin_val >> nodeId;
        NodeClassList[1].emplace_back(nodeId);
    }
    Gin_val.close();

    std::ifstream Gin_test(testPath);
    assert(Gin_test.is_open());
    while(!Gin_test.eof())
    {
        int nodeId;
        Gin_test >> nodeId;
        NodeClassList[2].emplace_back(nodeId);
    }
    Gin_test.close();
}


// 构建数据块
/*
 * 平衡测试集，验证集，训练集，
 * 对于每个块𝐵𝑖，它被分配给得分最高的分区。𝑃𝑗是已经分配给分区𝑗的顶点集。𝐶𝐸[𝑗]是𝐵𝑖和𝑃𝑗之间的交叉边数，如果将𝐵𝑖被分配给𝑃𝑗，它将被消除
 * 块分配之前先进行排序，先处理大块
 */
void NodeBlockConstruct(unordered_map<int,vector<int>>& node_map,
                        vector<vector<int>>& NodeClassList,
                        vector<Block>& blocks,
                        vector<int>& fanout,
                        vector<int>& viewedTable){
    for(auto& nodeId : NodeClassList[0]){
        viewedTable[nodeId] = nodeId;
        Block block;
        block.NodeList.push_back({nodeId});
        block.nodeViewed[nodeId] = 1;
        block.srcNodeID = nodeId;
        vector<int> tmp;
        for(auto& sampleId : node_map[nodeId]){
           if(viewedTable[sampleId] == -1){
               tmp.push_back(sampleId);
               viewedTable[sampleId] = nodeId;
               block.nodeViewed[sampleId] = 1;
           } else {
               // 该点已经被其他节点访问过
               if(block.nodeViewed.find(sampleId) == block.nodeViewed.end()){
                   block.holoNode.push_back(sampleId);
                   block.nodeViewed[sampleId] = 1;
               }
           }
        }
        if (!tmp.empty())
            block.NodeList.push_back(tmp);
        blocks.emplace_back(block);
    }

    // 继续使用BFS
    vector<int> activeBlocksTable(blocks.size(),0);
    for(int i = 0 ; i < blocks.size() ; i++){
        activeBlocksTable[i] = i;
    }
    bool blocksActive = true;
    while(blocksActive)
    {
        blocksActive = false;
        for(auto& blockId : activeBlocksTable){
            if(blockId == -1)
                continue;
            bool flag = false;
            // 读取最后一层
            vector<int> tmp;
            Block& block = blocks[blockId];
            for(auto& sampleId : block.NodeList.back()){
                for(auto& nextLayerId : node_map[sampleId]){
                    if(viewedTable[nextLayerId] == -1){
                        tmp.push_back(nextLayerId);
                        viewedTable[nextLayerId] = block.srcNodeID;
                        block.nodeViewed[nextLayerId] = 1;
                        flag = true;
                    } else {
                        if (block.holoLayer >= block.NodeList.size()&&block.nodeViewed.find(nextLayerId) == block.nodeViewed.end()) {
                            block.holoNode.push_back(nextLayerId);
                            block.nodeViewed[nextLayerId] = 1;
                        }
                    }
                }
            }
            if (flag){
                block.NodeList.push_back(tmp);
                blocksActive = true;
            } else
                blockId = -1;
        }
    }

    int index = 0;
    for(auto& block : blocks){block.refactorBlock(index++);}
}

void BlockSort(vector<Block>& blocks)
{
    // big -> small
    sort(blocks.begin(), blocks.end(),
         [](const auto& a, const auto& b)->bool { return a.BlockSize > b.BlockSize; });
}

float crossEdge()
{
    // TODO 实现
}


// 数据块连接分区
void BlockToPartition(vector<Block>& blocks,vector<Partition>& partitions, int partNumber){
    // 得到所有的block

    // TODO : 修改为初始化函数
    BlockSort(blocks);
    Partition partition;
    partitions.resize(partNumber,partition);
    for (int i = 0; i < partNumber; ++i) {
        partitions[i].partitionId = i;
    }
    partitions[0].mergeBlock(blocks[0]);
    partitions[0].mergeBlock(blocks[2]);
    partitions[1].mergeBlock(blocks[1]);
    partitions[1].mergeBlock(blocks[3]);
    //    float a = 0.6 , b = 0.2 , c = 0.2;
//    float maxAns = INT_MIN ;
//    int maxId = -1;
//    for(auto& block : blocks){
//        for(auto& partition : partitions){
//            float CE = crossEdge() / float (partition.size);
//            float BS = 1 - a* float(partition.trainSize) / float(partition.maxTrainSize)
//                    - b * float(partition.valSize) / float(partition.maxValSize)
//                    - c * float(partition.testSize )/ float(partition.maxTestSize);
//            float ans = CE * BS;
//            if(ans > maxAns){
//                maxAns = ans;
//                maxId = partition.partitionId;
//            }
//        }
//        // TODO:block合并
//    }
}

int main(int argc,char* argv[])
{
    int nodeNumber = 0;
    vector<int> viewedTable;
    unordered_map<int,vector<int>> node_map;
    vector<vector<int>> NodeClassList;
    vector<Block> blocks;
    vector<Partition> partitions;
    vector<int> fanout={25,10};

    //string filePath = "./" + string(argv[1]);
    string filePath = "../../data";
    fileToGraph(filePath,node_map);
    cout<<"Graph data read success!" <<endl;
    NodeClassList.push_back({0,5,11,17});
// txt文件命名规则
// name.txt,name_train.txt,name_val.txt,name_test.txt
// 读取训练集，测试集，验证集
//    readNodeClass(filePath,NodeClassList);
//    cout<<"Graph node class read success!"<<endl;
    nodeNumber = node_map.size();
    viewedTable.resize(nodeNumber,-1);
    NodeBlockConstruct(node_map,NodeClassList,blocks,fanout,viewedTable);

    BlockToPartition(blocks,partitions,2);
    return 0;
}

