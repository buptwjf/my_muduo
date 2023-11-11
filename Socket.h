//
// Created by 86188 on 2023/9/4.
//
#pragma once

#include "noncopyable.h"

class InetAddress;

class Socket : public noncopyable {
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}

    ~Socket();

    void bindAddress(const InetAddress &localaddr);

    int accept(InetAddress *peeraddr);

    void shutdownWrite() const;


    void listen();

    int accpet(InetAddress *peeraddr);

    void setTcpNoDelay(bool on);

    void setReuseAddr(bool on) const;

    void setReusePort(bool on) const;

    void setKeepAlive(bool on) const;

    int fd() const { return sockfd_; }


private:
    const int sockfd_;
};