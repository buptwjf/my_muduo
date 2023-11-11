//
// Created by 86188 on 2023/8/29.
//
#pragma once

#include "Poller.h"
#include "vector"
#include "sys/epoll.h"

/*
 * epoll 的使用
 * epoll_create
 * epoll_ctl    add/mod/del
 * epoll_wait
 * */
class EPollPoller : public Poller {
public:
    // epoll create
    EPollPoller(EventLoop *loop);

    ~EPollPoller() override;

    // 重写基类 Poller 的抽象方法
    Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;

    // epoll ctl 的行为
    void updateChannel(Channel *channel) override;

    void removeChannel(Channel *channel) override;

private:

    static const int kInitEventListSize = 16;

    // 填写活跃的连接
    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

    // 更新 channel 的通道
    void update(int operation, Channel *channel);

    using EventList = std::vector<epoll_event>;

    int epollfd_;
    EventList events_;
};