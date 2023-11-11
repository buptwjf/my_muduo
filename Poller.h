//
// Created by 86188 on 2023/8/29.
//
#pragma once

#include "noncopyable.h"
#include <vector>
#include <unordered_map>
#include "Timestamp.h"

class Channel;
class EventLoop;

// muduo 多路事件分发器的核心 IO 复用模块
class Poller : noncopyable {
public:
    using ChannelList = std::vector<Channel *>;

    explicit Poller(EventLoop *Loop);

    virtual ~Poller() = default;

    // 给所有 IO 复用保留的统一接口
    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;

    // epoll ctl
    virtual void updateChannel(Channel *channel) = 0;

    // epoll ctl 中 delete
    virtual void removeChannel(Channel *channel) = 0;

    // 判断一个 poller 中是否有某个 channel
    bool hasChannel(Channel *channel) const;

    // EventLoop 可以通过该接口获取默认的IO复用的具体实现
    static Poller *newDefaultPoller(EventLoop *loop);

protected:
    // map的  key sockfd  value  sockfd 所属的channel 通道类型
    using ChannelMap = std::unordered_map<int, Channel *>;
    ChannelMap channels_;

private:
    EventLoop *ownerLoop_; // 定义 Poller 所属的事件循环 EventLoop
};



