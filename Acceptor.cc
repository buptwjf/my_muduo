//
// Created by 86188 on 2023/9/5.
//
#include "Acceptor.h"
#include "Logger.h"
#include "InetAddress.h"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

static int createNonblocking() { // 创建一个非阻塞 IO
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0) {
        LOG_FATAL("%s:%s:%d listen socket create err:%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport) :
        loop_(loop),
        acceptSocket_(createNonblocking()),
        acceptChannel_(loop, acceptSocket_.fd()),
        listening_(false) {
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenAddr); // bind socket
    // TcpServer::start() Acceptor.listen  有新用户的连接，要执行一个回调（connfd=》channel=》subloop）
    // baseLoop => acceptChannel_(listenfd) =>
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
    acceptChannel_.remove();

}

void Acceptor::listen() {
    listening_ = true;
    acceptSocket_.listen(); // listen
    acceptChannel_.enableReading(); //accpetChannel_ => Poller
}

// listenfd 有事发生了，就是有新用户连接了
void Acceptor::handleRead() {
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0) {
        if (newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddr); // 轮询找到 subLoop，唤醒，分发当前客户端的 Channel
        } else { // 对应当前事件，没有回调
            ::close(connfd);
        }
    } else {
        LOG_ERROR("%s:%s:%d accept err:%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
        if (errno == EMFILE) {
            // 单台服务器不能满足部署了
            LOG_ERROR("%s:%s:%d sockfd reacher limit! \n", __FILE__, __FUNCTION__, __LINE__);
        }
    }


}

