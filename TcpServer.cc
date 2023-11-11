#include "TcpServer.h"
#include "Logger.h"

// 不接受用户传一个空指针
static EventLoop *CheckLoopNotNull(EventLoop *loop) {
    if (loop == nullptr) {
        LOG_FATAL("%s:%s:%d mainLoop is null!\n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg,
                     TcpServer::Option option) :
        loop_(CheckLoopNotNull(loop)),
        ipPort_(listenAddr.toIpPort()),
        name_(nameArg),
        acceptor_(new Acceptor(loop, listenAddr, option == kNoReusePort)),
        threadPool_(new EventLoopThreadPool(loop, name_)),
        connectionCallback_(),
        nextConnId_(1),
        started_(0) {
    // 当有用户连接时，会执行 TcpServer::newConnection回调
    acceptor_->setNewConnectionCallback(
            std::bind(&TcpServer::newConnection, this,
                      std::placeholders::_1, std::placeholders::_2));
}


TcpServer::~TcpServer() {

}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
    // 轮询算法
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {

}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &coon) {

}

void TcpServer::setThreadNum(int numThreads) {
    threadPool_->setThreadNum(numThreads);

}
// 开启服务器监听 loop.loop()
void TcpServer::start() {
    if (started_++ == 0) { // 防止一个 TcpServer 对象被 start 多次
        threadPool_->start(threadInitCallback_); // 启动底层的 loop 线程池
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}