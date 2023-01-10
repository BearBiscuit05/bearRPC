//
// Created by 74203 on 2022/12/23.
//

#include "Block.h"
#include "Partition.h"
/*
 * 复现ByteGNN关于自定义分区的内容
 */

using namespace std;

// 读取图数据
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

// 边界扩展
/* 采用BFS进行扩展，每个原顶点ID为广播ID，其他只需要保存第一次广播接收到的顶点
 */
void nodeBFS(){}

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
        block.srcNodeID = nodeId;
        vector<int> tmp;
        for(auto& sampleId : node_map[nodeId]){
           if(viewedTable[sampleId] == -1){
               tmp.push_back(sampleId);
               viewedTable[sampleId] = nodeId;
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
            Block block = blocks[blockId];
            for(auto& sampleId : block.NodeList.back()){
                for(auto& nextLayerId : node_map[sampleId]){
                    if(viewedTable[nextLayerId] == -1){
                        tmp.push_back(nextLayerId);
                        viewedTable[sampleId] = block.srcNodeID;
                        flag = true;
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
}

void BlockSort(vector<Block>& blocks)
{
    sort(blocks.begin(), blocks.end(),
         [](const auto& a, const auto& b)->bool { return a.BlockSize > b.BlockSize; });
}

float crossEdge()
{
    // TODO 实现
}


// 数据块连接分区
void BlockToPartition(vector<Block>& blocks,vector<Partition>& partitions){
    // 得到所有的block
    BlockSort(blocks);
    float a = 0.6 , b = 0.2 , c = 0.2;
    float maxAns = INT_MIN ;
    int maxId = -1;
    for(auto& block : blocks){
        for(auto& partition : partitions){
            float CE = crossEdge() / float (partition.size);
            float BS = 1 - a* float(partition.trainSize) / float(partition.maxTrainSize)
                    - b * float(partition.valSize) / float(partition.maxValSize)
                    - c * float(partition.testSize )/ float(partition.maxTestSize);
            float ans = CE * BS;
            if(ans > maxAns){
                maxAns = ans;
                maxId = partition.partitionId;
            }
        }
        // TODO:block合并

    }
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

    string filePath = "./" + string(argv[1]);
    // 读取图数据
    fileToGraph(filePath,node_map);
    cout<<"Graph data read success!" <<endl;
    // txt文件命名规则
    // name.txt,name_train.txt,name_val.txt,name_test.txt
    // 读取训练集，测试集，验证集
    readNodeClass(filePath,NodeClassList);
    cout<<"Graph node class read success!"<<endl;

    viewedTable.resize(nodeNumber,-1);
    for(auto& trainId : NodeClassList[0]){
        viewedTable[trainId] = trainId;
    }

    // 构建顶点方块
    NodeBlockConstruct(node_map,NodeClassList,blocks,fanout,viewedTable);

    // 构建方块分区
    BlockToPartition(blocks,partitions);
    return 0;
}

