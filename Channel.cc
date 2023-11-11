#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"
#include <assert.h>
#include <poll.h>
#include <sstream>
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

// EventLoop 包括 ChannelList Poller
Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1),
      tied_(false) // 采用简化版本
//   eventHandling_(false), addedToLoop_(false) {}
{}

Channel::~Channel() {
    // 判断由当前事件循环所在的线程析构 Channel
    /*
        省略部分代码
     */
}

// channel 的tie 支持什么时候调用
void Channel::tie(const std::shared_ptr<void> &obj) {
    tie_ = obj;
    tied_ = true;
}

// 当改变 channel 所表示 fd 的events事件后， update 负责在 poller 里面更改
// fd相应的事件
// epoll_ctl EventLoop=> ChannelList Poller
void Channel::update() {
    // 通过 Channel 所属的 EventLoop，用 poller的相应方法，注册 fd 的 events

    // add code
    // loop_->updateChannel(this); 暂未定义
}

// 在 channel 所属的 EventLoop中，把当前的 channel 删除掉
void Channel::remove() {
    // assert(isNoneEvent());
    // addedToLoop_ = false

    // add code
    // loop_->removeChannel;
}

void Channel::handleEvent(Timestamp receiveTime) {
    std::shared_ptr<void> guard;
    if (tied_) {
        guard = tie_.lock(); // 提升成强智能指针
        if (guard) {         // 提升成功
            handleEventWithGuard(receiveTime);
        }
    } else {
        handleEventWithGuard(receiveTime);
    }
}

// 根据poller 通知的 channel 发生的具体事件，由channel 负责具体的回调操作： 读
// 写 操作 关闭
void Channel::handleEventWithGuard(Timestamp receiveTime) {
    LOG_INFO("channel handleEvent revents: %d", revents_);
    // 省略部分
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
        if (closeCallBack_) {
            closeCallBack_();
        }
    }

    if (revents_ & (POLLERR | POLLNVAL)) {
        if (errorCallBack_) {
            errorCallBack_();
        }
    }

    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (readCallBack_) {
            readCallBack_(receiveTime);
        }
    }
    if (revents_ & POLLOUT) {
        if (writeCallBack_) {
            writeCallBack_();
        }
    }
}