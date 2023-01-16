#pragma once
#include "utils.h"
#include "RPCException.h"
#include <jackson/FileReadStream.h>
#include <jackson/Document.h>
#include <jackson/Writer.h>
#include <jackson/FileWriteStream.h>
#include <jackson/Value.h>
#include <jackson/StringWriteStream.h>

template <typename ProtocolServer>
class BaseServer
{
public:
    void setNumThread(size_t n) { server_.setNumThread(n); }
    void start() { server_.start(); }

protected:
    BaseServer(EventLoop* loop, const InetAddress& listen);
    ~BaseServer() = default;

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer& buffer);
    void onHighWatermark(const TcpConnectionPtr& conn, size_t mark);
    void onWriteComplete(const TcpConnectionPtr& conn);
    void handleMessage(const TcpConnectionPtr& conn, Buffer& buffer);
    void sendResponse(const TcpConnectionPtr& conn, const json::Value& response);

    ProtocolServer& convert();
    const ProtocolServer& convert() const;

protected:
    json::Value wrapException(RequestException& e);

private:
    TcpServer server_;
};

class RpcService
{
public:
    void addProcedureReturn(std::string_view methodName, ProcedureReturn* p)
    {
        assert(procedureReturn_.find(methodName) == procedureReturn_.end());
        procedureReturn_.emplace(methodName, p);
    }

    void addProcedureNotify(std::string_view methodName, ProcedureNotify *p)
    {
        assert(procedureNotfiy_.find(methodName) == procedureNotfiy_.end());
        procedureNotfiy_.emplace(methodName, p);
    }

    void callProcedureReturn(std::string_view methodName,
                             json::Value& request,
                             const RpcCallback& done);
    void callProcedureNotify(std::string_view methodName, json::Value& request);

private:
    typedef std::unique_ptr<ProcedureReturn> ProcedureReturnPtr;
    typedef std::unique_ptr<ProcedureNotify> ProcedureNotifyPtr;
    typedef std::unordered_map<std::string_view, ProcedureReturnPtr> ProcedureReturnList;
    typedef std::unordered_map<std::string_view, ProcedureNotifyPtr> ProcedureNotifyList;

    ProcedureReturnList procedureReturn_;
    ProcedureNotifyList procedureNotfiy_;
};


typedef std::function<void(json::Value&, const RpcCallback&)> ProcedureReturnCallback;
typedef std::function<void(json::Value&)> ProcedureNotifyCallback;

template <typename Func>
class Procedure
{
public:
    template<typename... ParamNameAndTypes>
    explicit Procedure(Func&& callback, ParamNameAndTypes&&... nameAndTypes):
            callback_(std::forward<Func>(callback))
    {
        constexpr int n = sizeof...(nameAndTypes);
        static_assert(n % 2 == 0, "procedure must have param name and type pairs");

        if constexpr (n > 0)
            initProcedure(nameAndTypes...);
    }

    // procedure call
    void invoke(json::Value& request, const RpcDoneCallback& done);
    // procedure notify
    void invoke(json::Value& request);

private:
    template<typename Name, typename... ParamNameAndTypes>
    void initProcedure(Name paramName, json::ValueType parmType, ParamNameAndTypes &&... nameAndTypes)
    {
        static_assert(std::is_same_v<Name, const char *> ||
                      std::is_same_v<Name, std::string_view>,
                      "param name must be 'const char*' or 'std::string_view'");
        params_.emplace_back(paramName, parmType);
        if constexpr (sizeof...(ParamNameAndTypes) > 0)
            initProcedure(nameAndTypes...);
    }

    template<typename Name, typename Type, typename... ParamNameAndTypes>
    void initProcedure(Name paramName, Type parmType, ParamNameAndTypes &&... nameAndTypes)
    {
        static_assert(std::is_same_v<Type, json::ValueType>, "param type must be json::ValueType");
    }

    void validateRequest(json::Value& request) const;
    bool validateGeneric(json::Value& request) const;

    struct Param
    {
        Param(std::string_view paramName_, json::ValueType paramType_) :
                paramName(paramName_),paramType(paramType_) {}

        std::string_view paramName;
        json::ValueType paramType;
    };

    Func callback_;
    std::vector<Param> params_;
}

typedef Procedure<ProcedureReturnCallback> ProcedureReturn;
typedef Procedure<ProcedureNotifyCallback> ProcedureNotify;

class RpcServer: public BaseServer<RpcServer>
{
public:
    RpcServer(EventLoop* loop, const InetAddress& listen):
        BaseServer(loop, listen){}
    ~RpcServer() = default;

    void addService(std::string_view serviceName, RpcService* service);

    void handleRequest(const std::string& json,
                    const RpcCallback& done);
    
private:
    void handleSingleRequest(json::Value& request,
                             const RpcCallback& done);
    void handleBatchRequests(json::Value& requests,
                             const RpcCallback& done);
    void handleSingleNotify(json::Value& request);

    void validateRequest(json::Value& request);
    void validateNotify(json::Value& request);

    typedef std::unique_ptr<RpcService> RpcServeicPtr;
    typedef std::unordered_map<std::string_view, RpcServeicPtr> ServiceList;

    ServiceList services_;
}