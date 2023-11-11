//
// Created by 86188 on 2023/8/29.
//
#include "Poller.h"
#include "Channel.h"

Poller::Poller(EventLoop *loop) : ownerLoop_(loop) {
}

bool Poller::hasChannel(Channel *channel) const {
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}

// 不应该写到这，因为需要引用 派生类的头文件
// #include "PollPoller.h"
// #include "EpollPoller.h"
//static Poller *newDefaultPoller(EventLoop *loop);
