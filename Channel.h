#pragma once

#include "Timestamp.h"
#include "noncopyable.h"
#include <functional>
#include <memory>

/*
    理清楚 EventLoop、Channel、Poller之间的关系  <= Reactor 模型上对应的
   Demultiplex 多路事件分发器 Channel 理解为通道。封装了 sockfd 和其感兴趣的
   event, 如 EPOLLIN、 EPOLLOUT 事件 还绑定了 poller 返回的具体事件
 */

class EventLoop; // 头文件中只用声明，源文件中 include
class Timestamp;

class Channel : noncopyable {

public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    explicit Channel(EventLoop *loop,
                     int fd); // 只用了 EeventLoop 的指针因此不需要包含源文件
    ~Channel();

    // fd 得到 poller 通知以后，处理事件。调用相应的回调方法
    void handleEvent(Timestamp receiveTime); // 需要包含 Timestamp 源文件

    // 设置回调函数对象
    void setReadCallback(ReadEventCallback cb) {
        readCallBack_ = std::move(cb);
    }

    void setWriteCallback(EventCallback cb) { writeCallBack_ = std::move(cb); }

    void setCloseCallback(EventCallback cb) { closeCallBack_ = std::move(cb); }

    void setErrorCallback(EventCallback cb) { errorCallBack_ = std::move(cb); }

    // 防止当 channel 被手动 remove 掉，channel
    // 还在执行回调操作，用一个弱智能指针监听 channel
    void tie(const std::shared_ptr<void> &);

    int fd() const { return fd_; }

    int events() const { return events_; }

    void set_revents(int revt) { revents_ = revt; }

    bool isNoneEvent() const { return events_ == kNoneEvent; }

    // 设置 fd 相应的事件状态
    void enableReading() {
        events_ |= kReadEvent;
        update();
    }

    void disableReading() {
        events_ &= ~kReadEvent;
        update();
    }

    void enableWriting() {
        events_ |= kWriteEvent;
        update();
    }

    void disableWriteing() {
        events_ &= ~kWriteEvent;
        update();
    }

    void disableAll() {
        events_ = kNoneEvent;
        update();
    }

    // 返回 fd 当前的事件状态
    bool isWriting() const { return events_ == kWriteEvent; }

    bool isReading() const { return events_ == kReadEvent; }

    int index() { return index_; }

    void set_index(int idx) { index_ = idx; }

    // one loop per thread
    // 当前 channel 属于哪个 EventLoop
    EventLoop *ownerLoop() { return loop_; }

    void remove();

private:
    void update();


    void handleEventWithGuard(Timestamp receiveTime);

    // 表示当前感兴趣事件 的描述
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *loop_; // 事件循环
    const int fd_;    // fd, Poller 监听的对象
    int events_;      // 注册fd 感兴趣的事件
    int revents_;     // poller 返回的具体发生事件
    int index_;

    std::weak_ptr<void> tie_;
    bool tied_;
    bool eventHandling_; // 暂未使用
    bool addedToLoop_;   // 暂未使用

    // 因为 channel 通道里面能获得 fd 最终发生的具体的事件
    // revents,所以他负责具体时间的回调操作
    ReadEventCallback readCallBack_;
    EventCallback writeCallBack_;
    EventCallback closeCallBack_;
    EventCallback errorCallBack_;
};
