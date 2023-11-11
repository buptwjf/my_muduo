#pragma once


#include "noncopyable.h"
#include "Timestamp.h"
#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>
#include "CurrentThread.h"

class Channel;

class Poller;

// 事件循环类 主要包含了两个大模块 Channel Poller (epoll的抽象) （muduo既有 pool 又有 epool）
class EventLoop : public noncopyable {
public:

    using Functor = std::function<void()>;

    EventLoop();

    ~EventLoop();

    // 开启事件循环
    void loop();

    // 退出事件循环
    void quit();


    Timestamp pollReturnTime() const { return pollReturnTime_; }

    // 在当前 loop 中执行 cb
    void runInLoop(Functor cb);

    // 把 cb 放入队列中，唤醒 loop 所在的线程，执行 cb
    void queueInLoop(Functor cb);

    // 唤醒 loop 所在的线程
    void wakeup() const;


    // EventLoop 的方法 -> Poller 的方法
    void updateChannel(Channel *channel);

    void removeChannel(Channel *channel);

    bool hasChannel(Channel *channel);

    // 判断 EventLoop 对象是否在自己的线程中
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

private:
    void abortNotInLoopThread();

    void handleRead() const;

    void doPendingFunctors();

    using ChannelList = std::vector<Channel *>;

    std::atomic_bool looping_;  // 原子操作，通过 CAS 实现
    std::atomic_bool quit_;     // 标识退出 loop 循环

    const pid_t threadId_; // 记录当前 loop 所在的线程 id


    Timestamp pollReturnTime_; // poller 返回发生事件的 channels 的时间点
    std::unique_ptr<Poller> poller_; // Eventloop 管理的资源

    int wakeupFd_; // !! 很重要，是 mainReactor (mainLoop) 通过轮询算法，选择一个 subLoop, 通过该成员唤醒 subReactor 的方式
    std::unique_ptr<Channel> wakeupChannel_;

    // context_

    ChannelList activeChannels_;
//    Channel *currentActiveChannel_;

//    mutable MutexLock mutex;
    std::atomic_bool callingPendingFunctors_; // 标识当前 loop 是否有需要执行的回调操作
    std::vector<Functor> pendingFunctors_; // 存储loop 需要执行的所有回调操作
    std::mutex mutex_;  // 互斥锁，用来保护上面vector容器的线程安全操作
};

