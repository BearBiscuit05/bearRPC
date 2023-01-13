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