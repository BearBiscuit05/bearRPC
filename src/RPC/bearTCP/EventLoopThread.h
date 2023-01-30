#pragma once
#include <thread>

#include <bearTCP/CountDownLatch.h>

namespace ev
{

class EventLoop;

class EventLoopThread: noncopyable
{
public:
    EventLoopThread() = default;
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void runInThread();

    bool started_ = false;
    EventLoop* loop_ = nullptr;
    std::thread thread_;
    CountDownLatch latch_{1};
};

}