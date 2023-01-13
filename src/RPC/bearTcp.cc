#include "bearTcp.h"

namespace
{
const size_t kHighWatermark = 65536;
const size_t kMaxMessageLen = 100 * 1024 * 1024;
}

template <typename ProtocolServer>
BaseServer<ProtocolServer>::BaseServer(EventLoop *loop, const InetAddress& listen)
    :server_(loop, listen)
{
    server_.setConnectionCallback(std::bind(
            &BaseServer::onConnection, this, _1));
    server_.setMessageCallback(std::bind(
            &BaseServer::onMessage, this, _1, _2));
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected()) {
        DEBUG("connection %s is [up]",
              conn->peer().toIpPort().c_str());
        conn->setHighWaterMarkCallback(std::bind(
                &BaseServer::onHighWatermark, this, _1, _2), kHighWatermark);
    }
    else {
        DEBUG("connection %s is [down]",
              conn->peer().toIpPort().c_str());
    }
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::onMessage(const TcpConnectionPtr& conn, Buffer& buffer)
{
    handleMessage(conn, buffer);
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::onHighWatermark(const TcpConnectionPtr& conn, size_t mark)
{
    DEBUG("connection %s high watermark %lu",
         conn->peer().toIpPort().c_str(), mark);

    conn->setWriteCompleteCallback(std::bind(
            &BaseServer::onWriteComplete, this, _1));
    conn->stopRead();
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::onWriteComplete(const TcpConnectionPtr& conn)
{
    DEBUG("connection %s write complete",
         conn->peer().toIpPort().c_str());
    conn->startRead();
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::sendResponse(const TcpConnectionPtr& conn, const json::Value& response)
{
    // 序列化 将json转移为string
    json::StringWriteStream os;
    json::Writer writer(os);
    response.writeTo(writer);

    // wish sso string don't allocate heap memory...
    auto message = std::to_string(os.get().length() + 2)
            .append("\r\n")
            .append(os.get())
            .append("\r\n");
    conn->send(message);
}