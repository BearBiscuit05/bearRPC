#pragma once

#include <bearTCP/noncopyable.h>
#include <bearTCP/InetAddress.h>
#include <bearTCP/Channel.h>

namespace bear
{
class EventLoop;

class Acceptor:noncopyable
{
    Acceptor(EventLoop* loop, const InetAddress& local);
    ~Acceptor();

    bool listening() const
    { return listening_; }

    void listen();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { newConnectionCallback_ = cb; }

private:
    void handleRead();

    bool listening_;
    EventLoop* loop_;
    const int acceptFd_;
    Channel acceptChannel_;
    InetAddress local_;
    NewConnectionCallback newConnectionCallback_;    
};

}