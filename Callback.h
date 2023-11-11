//
// Created by 86188 on 2023/9/5.
//

#pragma once


#include <memory>
#include <functional>

class Buffer;

class TcpConnection;

class Timestamp;

// TcpServer 相关
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
using MessageCallback = std::function<void(const TcpConnectionPtr &, Buffer *, Timestamp)>;

// TcpConnection 相关
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr &, size_t)>;
using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
