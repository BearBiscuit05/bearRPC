#pragma once
#include <string>
#include <string_view>

#include <jackson/Value.h>

#include <tinyev/EventLoop.h>
#include <tinyev/TcpConnection.h>
#include <tinyev/TcpServer.h>
#include <tinyev/TcpClient.h>
#include <tinyev/InetAddress.h>
#include <tinyev/Buffer.h>
#include <tinyev/Logger.h>
#include <tinyev/Callbacks.h>
#include <tinyev/Timestamp.h>
#include <tinyev/ThreadPool.h>
#include <tinyev/CountDownLatch.h>

typedef std::function<void(json::Value response)> RpcCallback;

using ev::EventLoop;
using ev::TcpConnection;
using ev::TcpServer;
using ev::TcpClient;
using ev::InetAddress;
using ev::TcpConnectionPtr;
using ev::noncopyable;
using ev::Buffer;
using ev::ConnectionCallback;
using ev::ThreadPool;
using ev::CountDownLatch;

class UserCallback
{
private:
    mutable json::Value request_;
    RpcCallback callback_;

public:
    UserCallback(json::Value &request,
                     const RpcCallback &callback)
            : request_(request),
              callback_(callback)
    {}


    void operator()(json::Value &&result) const
    {
        json::Value response(json::TYPE_OBJECT);
        response.addMember("jsonrpc", "2.0");
        response.addMember("id", request_["id"]);
        response.addMember("result", result);
        callback_(response);
    }
};