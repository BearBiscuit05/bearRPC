#pragma once
#include <memory>
#include <set>

#include <bearTCP/Timer.h>
#include <bearTCP/Channel.h>

namespace bear
{

class TimerQueue: noncopyable
{
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    Timer* addTimer(TimerCallback cb, Timestamp when, Nanosecond interval);
    void cancelTimer(Timer* timer);

private:
    typedef std::pair<Timestamp, Timer*> Entry;
    typedef std::set<Entry> TimerList;

    void handleRead();
    std::vector<Entry> getExpired(Timestamp now);

private:
    EventLoop* loop_;
    const int timerfd_;
    Channel timerChannel_;
    TimerList timers_;
}

}