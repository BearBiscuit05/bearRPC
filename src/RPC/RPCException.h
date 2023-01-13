#pragma once
#include <cassert>
#include <vector>
#include <cstdint>
#include <exception>
#include <jackson/Value.h>
#include <string>
#include <memory>

enum Error
{
    RPC_PARSE_ERROR,
    RPC_INVALID_REQUEST,
    RPC_METHOD_NOT_FOUND,
    RPC_INVALID_PARAMS,
    RPC_INTERNAL_ERROR,
};

class RpcError
{
public:
    RpcError(Error err):err_(err){}
    explicit RpcError(int errorCode):
            err_(fromErrorCode(errorCode)){}

    const char* asString() const
    { return errorString[err_]; }

    int asCode() const
    { return errorCode[err_]; }


private:
    const Error err_;
    static Error fromErrorCode(int code)
    {
        switch(code) {
            case -32700: return RPC_PARSE_ERROR;
            case -32600: return RPC_INVALID_REQUEST;
            case -32601: return RPC_METHOD_NOT_FOUND;
            case -32602: return RPC_INVALID_PARAMS;
            case -32603: return RPC_INTERNAL_ERROR;
            default: assert(false && "bad error code");
        }
    }

    static inline const std::vector<int> errorCode = {-32700,-32600,-32601,-32602,-32603};
    static inline const std::vector<char*> errorString = {"Parse error","Invalid request","Method not found","Invalid params","Internal error"};
};

class NotifyException : public std::exception
{
public:
    explicit NotifyException(RpcError err, const std::string detail):
            err_(err),detail_(detail){}
    const char* what() const noexcept{return err_.asString();}
    RpcError err() const{return err_;}
    const std::string detail() const{return detail_;}
private:
    RpcError err_;
    const std::string detail_;
};

class RequestException: public std::exception
{
    RequestException(RpcError err, json::Value id, const std::string detail):
            err_(err),id_(new json::Value(id)),detail_(detail){}
    
    explicit RequestException(RpcError err, const std::string detail):
            err_(err),id_(new json::Value(json::TYPE_NULL)),detail_(detail){}


    const char* what() const noexcept{return err_.asString();}
    RpcError err() const{return err_;}
    json::Value& id(){return *id_;}
    const std::string detail() const{return detail_;}

private:
    RpcError err_;
    std::unique_ptr<json::Value> id_;
    const std::string detail_;
};

class ResponseException: public std::exception
{
public:
    explicit ResponseException(const std::string msg):
            hasId_(false),id_(-1),msg_(msg){}
    
    ResponseException(const std::string msg, int id):
            hasId_(true),id_(id),msg_(msg){}

    const char* what() const noexcept{return (char*)msg_.c_str();}
    bool hasId() const{return hasId_;}
    int Id() const{return id_;}

private:
    const bool hasId_;
    const int id_;
    const std::string msg_;
};

class StubException: std::exception
{
public:
    explicit StubException(const std::string msg):msg_(msg){}
    const char* what() const noexcept{return (char*)msg_.c_str();}
private:
    const std::string msg_;
};