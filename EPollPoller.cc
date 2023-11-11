//
// Created by 86188 on 2023/8/29.
//

#include "EPollPoller.h"
#include "Logger.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include "assert.h"
#include "Channel.h"
//#include "cstring"
#include "string.h"

// 标识 Epoll 和 Channel 的状态
// channel 未添加到 poller 中
const int kNew = -1;    // 未添加 channel 的成员 index_ = -1
// channel 已添加到 poller 中
const int kAdded = 1;
// channel 已从 poller 中删除
const int kDeleted = 2; // 删除

// 对应 epoll_create
EPollPoller::EPollPoller(EventLoop *loop) : Poller(loop), epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
                                            events_(kInitEventListSize) {
    if (epollfd_ < 0) {
        LOG_FATAL("epoll_create error: %d \n", errno);
    }
}

// 对应 epoll_close
EPollPoller::~EPollPoller() {
    ::close(epollfd_);
}

// 填写活跃的连接
void EPollPoller::fillActiveChannels(int numEvents, Poller::ChannelList *activeChannels) const {
//    assert(implict_cast<size_t>(numEvents) <= events_.size())
    /* 省略部分代码 */
    for (int i = 0; i < numEvents; ++i) {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);  // EventLoop 就拿到了它的 poller 给它返回的所有发生事件的 channel 列表了
    }
}


// 更新 channel 通道
void EPollPoller::update(int operation, Channel *channel) {
    // 创建 event
    struct epoll_event event;
    // 清零
//    memZero(&event, sizeof event);
    bzero(&event, sizeof(event));
    memset(&event, 0, sizeof event);
    int fd = channel->fd();
    event.data.fd = fd;
    event.events = channel->events();
    event.data.ptr = channel;
    /* 省略 */
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) { // 如果仅仅是没有删掉
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        } else {
            LOG_FATAL("epoll_ctl add/mode error:%d\n", errno);
        }
    }
}

// 通过 epoll_wait  把发生事件的 channel 通过 activeChannels 发给 EventLoop
Timestamp EPollPoller::poll(int timeoutMs, Poller::ChannelList *activeChannels) {
    // 实际上用 LOG_DEBUG 输出日志更为合理
    LOG_INFO("func = %s = > fd total count: %lu\n", __FUNCTION__, channels_.size());
    int numEvents = ::epoll_wait(epollfd_,
                                 &*events_.begin(), // 拿到 vector 数组的起始地址
                                 static_cast<int>(events_.size()), // size_t 转为 int
                                 timeoutMs);
    int savedError = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        LOG_INFO("%d events happened\n", numEvents);
        fillActiveChannels(numEvents, activeChannels);
//        if (implicit_cast<size_t>(numEvents) == events_.size()) // 简化
        if (numEvents == events_.size()) { // 需要进行扩容
            events_.resize((events_.size() * 2));
        }
    } else if (numEvents == 0) {
//        LOG_DEBUG("%s timeout! \n", __FUNCTION__);
        LOG_INFO("%s timeout! \n", __FUNCTION__);

    } else {
        if (savedError != EINTR) { // 由其他的错误类型引起
            errno = savedError;  // 全局有可能其他 loop 把 errno 的值改变
            LOG_ERROR("EPollPoller::poll()");
        }
    }
    return now;

}

// 相当于 epoll_ctl
// channel 的 update remove => EventLoop undateChannel removeChannel => Poller => EpollPoller/*
//     *              EventLoop
//     *      ChannelList              Poller (EPoll 和 Poll)
//     *   所有的channel             >=      channelMap  <fd, channerl*> 注册过的 channel
//     channel 无法直接调用 poller, 而是通过他的父类 调用
//     * */
void EPollPoller::updateChannel(Channel *channel) {
    // EPollPoller 的三种状态
    const int index = channel->index();
    //
    LOG_INFO("func = %s, fd=%d, events=%d, index= %d\n", __FUNCTION__, channel->fd(), channel->events(), index);
    if (index == kNew || index == kDeleted) { // 未注册过或者注册过被删除
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (index == kNew) {
            // 原本就没有
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        } else {
            // 已注册过，但是被删除
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }

        // 添加 channel
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel); // 添加一个 channel
    } else {
        // update existing one with EPOLL_CTL_MOF/DEL
        int fd = channel->fd();
//        (void) fd; //?
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent()) { // 已注册，但对任何时间都不敢兴趣
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel); //
        }
    }

}

// 相当于 epoll_ctl
// 从 Poller 中删除 channel
void EPollPoller::removeChannel(Channel *channel) {
    // assert
    int fd = channel->fd();
    //
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());

    int index = channel->index();
    // 必须是已经添加过的
    assert(index == kAdded || index == kDeleted);

    size_t n = channels_.erase(fd); // 删除这个 fd
    (void) n;
    assert(n == 1);
    if (index == kAdded) { // 必须从 epoll 中删除
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

