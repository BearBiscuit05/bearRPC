/*
    TODO:
    1.将信息从文件/流中读出
    2.将信息进行解析
    3.将信息传递到对应位置
*/
#include "Serialization.h"
using namespace std;
int main()
{
    auto value = readJsonFromFile("./document.json");
    parseProto(value);
    
    // json::FileWriteStream os(stdout);
    // json::Writer writer(os);
    // value.writeTo(writer);
    return 0;
}
