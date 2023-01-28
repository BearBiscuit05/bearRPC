#include <sys/timerfd.h>
#include <strings.h>
#include <unistd.h>
#include <ratio> // std::nano::den

#include <bearTCP/Logger.h>
#include <bearTCP/EventLoop.h>
#include <bearTCP/TimerQueue.h>

using namespace bear;

namespace
{

int timerfdCreate()
{   
    /*
        timerfd_create :函数创建一个定时器对象，同时返回一个与之相关联的文件描述符
        CLOCK_MONOTONIC:从系统启动这一刻起开始计时,不受系统时间被用户改变的影响
        TFD_NONBLOCK(非阻塞模式)/TFD_CLOEXEC(表示当程序执行exec函数时本fd将被系统自动关闭,表示不传递)
    */
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (fd == -1)
        SYSFATAL("timer_create()");
    return fd;
}

void timerfdRead(int fd)
{
    uint64_t val;
    ssize_t n = read(fd, &val, sizeof(val));
    if (n != sizeof(val))
        ERROR("timerfdRead get %ld, not %lu", n, sizeof(val));
}

struct timespec durationFromNow(Timestamp when)
{
    struct timespec ret;
    Nanosecond ns = when - clock::now();
    if (ns < 1ms) ns = 1ms;

    ret.tv_sec = static_cast<time_t>(ns.count() / std::nano::den);
    ret.tv_nsec = ns.count() % std::nano::den;
    return ret;
}

void timerfdSet(int fd, Timestamp when)
{
    
    struct itimerspec oldtime, newtime;
    bzero(&oldtime, sizeof(itimerspec));
    bzero(&newtime, sizeof(itimerspec));
    newtime.it_value = durationFromNow(when);
    /*
        timerfd_settime:此函数用于设置新的超时时间，并开始计时,能够启动和停止定时器;
        new_value:      参数new_value指定定时器的超时时间以及超时间隔时间
        old_value:      如果old_value不为NULL, old_vlaue返回之前定时器设置的超时时间
    */
    int ret = timerfd_settime(fd, 0, &newtime, &oldtime);
    if (ret == -1)
        SYSERR("timerfd_settime()");
}

}

TimerQueue::TimerQueue(EventLoop *loop)
        : loop_(loop),
          timerfd_(timerfdCreate()),
          timerChannel_(loop, timerfd_)
{
    loop_->assertInLoopThread();
    timerChannel_.setReadCallback([this](){handleRead();});
    timerChannel_.enableRead();
}

TimerQueue::~TimerQueue()
{
    for (auto& p: timers_)
        delete p.second;
    ::close(timerfd_);
}

Timer* TimerQueue::addTimer(TimerCallback cb, Timestamp when, Nanosecond interval)
{
    Timer* timer = new Timer(std::move(cb), when, interval);
    loop_->runInLoop([=](){
        auto ret = timers_.insert({when, timer});
        assert(ret.second);

        if (timers_.begin() == ret.first)
            timerfdSet(timerfd_, when);
    });
    return timer;
}

void TimerQueue::cancelTimer(Timer* timer)
{
    loop_->runInLoop([timer, this](){
        timer->cancel();
        timers_.erase({timer->when(), timer});
        delete timer;
    });
}

void TimerQueue::handleRead()
{
    loop_->assertInLoopThread();
    timerfdRead(timerfd_);

    Timestamp now(clock::now());
    for (auto& e: getExpired(now)) {
        Timer* timer = e.second;
        assert(timer->expired(now));

        if (!timer->canceled())
            timer->run();
        if (!timer->canceled() && timer->repeat()) {
            timer->restart();
            e.first = timer->when();
            timers_.insert(e);
        }
        else delete timer;
    }

    if (!timers_.empty())
        timerfdSet(timerfd_, timers_.begin()->first);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    Entry en(now + 1ns, nullptr);
    std::vector<Entry> entries;

    auto end = timers_.lower_bound(en);
    entries.assign(timers_.begin(), end);
    timers_.erase(timers_.begin(), end);

    return entries;
}

