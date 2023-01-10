#include <iostream>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <cassert>
#include <string>
#include <algorithm>
using namespace std;
void fileToGraph(string filepath, unordered_map<int,vector<int>>& node_map)
{
    std::ifstream Gin(filepath);
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

void fileToPartition(string filepath,unordered_map<int,vector<int>>& node_partition_map)
{
    std::ifstream Gin(filepath);
    assert(Gin.is_open());
    int nodeNumber = 0;
    while(!Gin.eof())
    {
        int partId;
        Gin >> partId;
        node_partition_map[nodeNumber++].push_back(partId);
    }
    Gin.close();
}

void fileToTrainList(string filepath, vector<int>& trainNodeList)
{
    std::ifstream Gin(filepath);
    assert(Gin.is_open());
    while(!Gin.eof())
    {
        int nodeId;
        Gin >> nodeId;
        trainNodeList.push_back(nodeId+100000);
        //trainNodeList.push_back(nodeId + rand() % 100000 - rand() % 9022);
    }
    Gin.close();
}

vector<int> sim_random_sample_list(vector<int> nodeIds,unordered_map<int,vector<int>>& node_map, int sampleNumber)
{
    srand(unsigned(time(NULL)));
    for(int nodeId : nodeIds){
        vector<int> sampleList = node_map[nodeId];
        if(sampleList.size() < sampleNumber){
            return sampleList;
        } else {
            random_shuffle(sampleList.begin(), sampleList.end());
            vector<int> copy(sampleList.begin(), sampleList.begin() + sampleNumber);
            return copy;
        }
    }
    return {};
}

void sample(
        vector<int>& trainId,
        vector<int>& fanout ,
        unordered_map<int,vector<int>>& node_map,
        unordered_map<int,vector<int>>& sample_list_with_Id)
{
    vector<vector<int>> neighbors;
    vector<int> tmp;
    for(int id : trainId){
        neighbors.clear();
        tmp.clear();
        tmp.push_back(id);
        neighbors.push_back(tmp);
        for(int i = 0 ; i < fanout.size(); i++){
            tmp = sim_random_sample_list(neighbors[i],node_map,fanout[i]);
            neighbors.push_back(tmp);
        }
        tmp.clear();
        for(vector<int> neighbor: neighbors){
            tmp.insert(tmp.end(),neighbor.begin(),neighbor.end());
        }
        sample_list_with_Id[id] = tmp;
    }
}

void countMsg(
        unordered_map<int,vector<int>>& sample_list_with_Id,
        unordered_map<int,vector<int>>& node_partition_map)
{
    int MsgCount = 0;
    for(auto& sampleList : sample_list_with_Id){
        int nodePartitionId = node_partition_map[sampleList.first][0];
        vector<int> sampled = sampleList.second;
        for(auto& sampleNode : sampled){
            vector<int> partIds = node_partition_map[sampleNode];
            if(find(partIds.begin(), partIds.end(), nodePartitionId) == partIds.end()){
                MsgCount++;
                node_partition_map[sampleNode].push_back(nodePartitionId);
            }
        }
    }
    cout << "MsgCount :" <<MsgCount <<endl;
}

void CountBalance(vector<int>& trainNodeList,unordered_map<int,vector<int>>& node_partition_map)
{
    unordered_map<int,int> balanceCount;
    for(auto& trainId : trainNodeList)
    {
        if (balanceCount.find(node_partition_map[trainId][0]) == balanceCount.end())
            balanceCount[node_partition_map[trainId][0]] = 0;
        balanceCount[node_partition_map[trainId][0]]++;
    }

    for (int i = 0; i < balanceCount.size(); ++i) {
        cout << "partition " << i << " has node number:" << balanceCount[i] <<endl;
    }
}

int main(int argc,char* argv[]) {
    unordered_map<int,vector<int>> node_map;
    unordered_map<int,vector<int>> node_partition_map;
    unordered_map<int,vector<int>> sample_list_with_Id;
    vector<int> trainNodeList;
    vector<int> fanout = {25,10};
    //string dataName = "product";
    cout << "test file path : " << argv[1] <<endl;
    fileToGraph(argv[1],node_map);
    cout<<"Graph file read success!!!"<<endl;
    fileToPartition(argv[2],node_partition_map);
    cout<<"Graph partition file read success!!!"<<endl;
    fileToTrainList(argv[3],trainNodeList);
    cout<<"Graph trainNode file read success!!!"<<endl;
    sample(trainNodeList,fanout,node_map,sample_list_with_Id);
    cout<<"sample finished !!!"<<endl;
    countMsg(sample_list_with_Id,node_partition_map);
    cout<<"sim finished !!!"<<endl;
    CountBalance(trainNodeList,node_partition_map);
    return 0;
}
