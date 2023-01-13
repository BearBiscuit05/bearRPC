/*
    TODO:
    1.将信息从文件/流中读出
    2.将信息进行解析
    3.将信息传递到对应位置
*/
#include "Serialization.h"
using namespace std;

inline void expect(bool result, const char* errMsg)
{
    if (!result) {
        throw errMsg;
    }
}

json::Value readJsonFromFile(char* filePath)
{
    FILE* input = fopen(filePath,"r");
    assert(input != nullptr);
    json::FileReadStream in(input);
    json::Document proto;
    auto err = proto.parseStream(in);
    fclose(input);
    return proto;
    
}

// msg解析
void parseProto(json::Value& proto)
{
    expect(proto.isObject(),
           "expect object");
    expect(proto.getSize() == 2,
           "expect 'name' and 'rpc' fields in object");

    auto name = proto.findMember("name");

    expect(name != proto.memberEnd(),
           "missing service name");
    expect(name->value.isString(),
           "service name must be string");
    //serviceInfo_.name = name->value.getString();

    auto rpc = proto.findMember("rpc");
    expect(rpc != proto.memberEnd(),
           "missing service rpc definition");
    expect(rpc->value.isArray(),
           "rpc field must be array");

    size_t n = rpc->value.getSize();
    for (size_t i = 0; i < n; i++) {
        parseRpc(rpc->value[i]);
    }
}

// RPC 解析
void parseRpc(json::Value& rpc)
{
    expect(rpc.isObject(),"rpc definition must be object");

    auto name = rpc.findMember("name");
    expect(name != rpc.memberEnd(),"missing name in rpc definition");
    expect(name->value.isString(),"rpc name must be string");

    auto params = rpc.findMember("params");
    bool hasParams = params != rpc.memberEnd();
    // if (hasParams) {
    //     validateParams(params->value);
    // }

    auto returns = rpc.findMember("returns");
    bool hasReturns = returns != rpc.memberEnd();
    // if (hasReturns) {
    //     validateReturns(returns->value);
    // }

    // auto paramsValue = hasParams ?
    //                    params->value :
    //                    json::Value(json::TYPE_OBJECT);

    // if (hasReturns) {
    //     RpcReturn r(name->value.getString(), paramsValue, returns->value);
    //     serviceInfo_.rpcReturn.push_back(r);
    // }
    // else {
    //     RpcNotify r(name->value.getString(), paramsValue);
    //     serviceInfo_.rpcNotify.push_back(r);
    // }
}