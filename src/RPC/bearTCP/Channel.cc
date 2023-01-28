#include <cassert>

#include <bearTCP/EventLoop.h>
#include <bearTCP/Channel.h>

using namespace bear;

Channel:Channel(EventLoop* loop, int fd)
        : polling(false),
          loop_(loop),
          fd_(fd),
          tied_(false),
          events_(0),
          revents_(0),
          handlingEvents_(false)
{}

Channel::~Channel()
{ assert(!handlingEvents_); }

void Channel::handleEvents()
{
    loop_->assertInLoopThread();
    // use weak_ptr->shared_ptr to extend life-time.
    if (tied_) {
        auto guard = tie_.lock();
        if (guard != nullptr)
            handleEventsWithGuard();
    }
    else handleEventsWithGuard();
}

void Channel::handleEventsWithGuard()
{
    /*
        EPOLLIN     表示对应文件描述符可读
        EPOLLOUT    表示对应文件描述符可写
        EPOLLHUP    表示对应文件描述符被挂断
        EPOLLPRI    表示对应的文件描述符有紧急的数据可读
        EPOLLERR    表示1对应文件描述符发生错误
    */
    handlingEvents_ = true;
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        if (closeCallback_) closeCallback_();
    }
    if (revents_ & EPOLLERR) {
        if (errorCallback_) errorCallback_();
    }
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (readCallback_) readCallback_();
    }
    if (revents_ & EPOLLOUT) {
        if (writeCallback_) writeCallback_();
    }
    handlingEvents_ = false;
}

void Channel::tie(const std::shared_ptr<void>& obj)
{
    tie_ = obj;
    tied_ = true;
}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::remove()
{
    assert(polling);
    loop_->removeChannel(this);
}