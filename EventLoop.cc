#include "EventLoop.h"
#include <sys/eventfd.h>
#include  "Logger.h"
#include "Poller.h"
#include "CurrentThread.h"
#include "Channel.h"
#include <functional>
#include <mutex>

namespace {
// 防止一个线程创建多个 EventLoop (一个线程只有一个 EventLoop)
    __thread EventLoop *t_loopInThisThread = nullptr;

// 定义默认Pollerr IO复用接口的超时时间
    const int kPollTimeMs = 10000;

// 创建 wakeupfd 用来唤醒 subReactor 处理新来的 channel
    int createEventfd() {
        int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (evtfd < 0) {
            LOG_FATAL("eventfd error: %d \n", errno);
        }
        return evtfd;
    }
}

EventLoop::EventLoop() :
        looping_(false),
        quit_(false),
        callingPendingFunctors_(false),
        threadId_(CurrentThread::tid()),
        poller_(Poller::newDefaultPoller((this))),
        wakeupFd_(createEventfd()),
        wakeupChannel_(new Channel(this, wakeupFd_)) {
//        currentActiveChannel_(nullptr) {
    LOG_DEBUG("EventLoop created %p in thread %d \n", this, threadId_);
    if (t_loopInThisThread) {
        LOG_FATAL("Another EventLoop %p , exists in this thread%d \n ", t_loopInThisThread, threadId_);
    } else {
        t_loopInThisThread = this;
    }

    // 设置 wakeupfd 的事件类型以及发生事件后的回调操作
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // 每一个 eventloop 都将监听 wakeupChannel的读事件
    wakeupChannel_->enableReading();

}

EventLoop::~EventLoop() {
    //
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

// 开始事件循环
void EventLoop::loop() {
    // 省略
    looping_ = true;
    quit_ = false;

    LOG_INFO("EventLoop %p start looping \n", this);
    while (!quit_) {
        activeChannels_.clear();
        // 监听两类 fd，一种是 client的fd，一种 wakeup_fd
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);

        for (Channel *channel: activeChannels_) {
            // Poller 监听哪些channel 发生事件了，然后上报给 EventLoop，通过channel 处理相应的事件
            channel->handleEvent(pollReturnTime_);
        }
        // 执行当前 EventLoop 事件循环所需要的回调操作
        /*
         * IO 线程 mainLoop 主要做的是 accept 的工作，返回一个和客户端通信的fd
         * channel 负责打包， 分发给 subloop(如果是四核的，就会启动3个 subloop)
         * mainLoop 事先注册一个回调 cb(需要 subloop 来执行) wakeup 以后
         * 执行下面的的方法， 执行之前 mainloop 注册的方法（可能是一个，也可能是多个）
         * */
        doPendingFunctors();
    }
    LOG_INFO("EventLoop %p stop looping.\n", this);
}


// 退出事件循环
// 1. loop 在自己的线程中 quit
/*
 *             mainLoop  负责放
 *                  muduo库中没有        ================生产者-消费者的线程安全的队列
 *                  而是通过 wakeupfd
 *  subLoop1   subLoop2    subLoop3   负责消费
 * */
void EventLoop::quit() {
    quit_ = true;
    // 2.如果是在其他线程中，调用 quit 在一个 subloop 中调用 mianloop 的quit
    // 在 subloop(worker) 的线程中调用 mainloop 的 quit，
    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(EventLoop::Functor cb) {
    if (isInLoopThread()) { // 在当前的 loop  线程中执行 cb
        cb();
    } else { // 不在当前 loop,加入 queue
        queueInLoop((std::move(cb))); //
    }
}

// 把 cb 放入队列中，唤醒 loop 所在的线程，执行 cb
void EventLoop::queueInLoop(EventLoop::Functor cb) {
    {
        std::unique_lock<std::mutex> look(mutex_);
        pendingFunctors_.emplace_back(cb);
    }
    // 唤醒相应的，需要执行上面回调操作的 loop 线程了
    if (!isInLoopThread() || callingPendingFunctors_) { // loop 不是这个线程的
        // callingPendingFunctors_ 确实在当前线程的 loop 中执行回调
        wakeup(); // 唤醒 loop 所在线程
    }
}

// 用来唤醒 loop 所在的线程，向 wakeup 写一个数据, wakeupChannel 就发生读事件，
// 当前 loop 线程就会被唤醒
void EventLoop::wakeup() const {
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR("EventLoop::wakeup() write %lu bytes instead of 8\n", n);
    }
}

void EventLoop::updateChannel(Channel *channel) {
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel) {
    return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread() {

}

void EventLoop::handleRead() const {
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof one) {
        LOG_ERROR("EventLoop::handlePread() reads %ld bytes instead of 8", n);
    }
}

// 执行回调，由于执行回调的过程中，mainloop还有可能下发新的回调
// 创建一个副本，从副本中执行回调，然后进行交换
void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    // 执行回调
    for (const Functor &functor: functors) {
        functor();  // 执行当前需要执行的回调操作
    }
    callingPendingFunctors_ = false;
}
