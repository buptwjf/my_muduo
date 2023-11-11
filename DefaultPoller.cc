//
// Created by 86188 on 2023/8/29.
//
#include "Poller.h"
#include "stdlib.h"
#include "EPollPoller.h"

Poller *Poller::newDefaultPoller(EventLoop *loop) {
    if (getenv("MUDUO_USE_POLL")) {
        return nullptr; // 生成 poll 实例
    } else {

        return new EPollPoller(loop); // 生成 epoll 实例
    }
}