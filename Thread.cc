//
// Created by 86188 on 2023/8/31.
//
#include "Thread.h"
#include "CurrentThread.h"
#include <semaphore.h>

// 类内静态成员变量必须在构造函数外单独初始化 !!
std::atomic_int32_t numCreated_(0);
//std::atomic_int32_t numCreated_;
Thread::Thread(ThreadFunc func, const std::string &name) :
        started_(false),
        joined_(false),
        tid_(0),
        func_(std::move(func)),
        name_(name) {
    setDefaultName();
}

Thread::~Thread() {
    if (started_ && !joined_) { // join 和 detach 不能一起
        thread_->detach(); // 直接使用 linux 库中的 thread 的线程分离方法
    }
}

// 启动一个线程
void Thread::start() { // 一个 Thread 对象，
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    // muduo 库用了一个 struct ThreadDate 来传递变量，这里进行了简化
    thread_ = std::shared_ptr<std::thread>(new std::thread([&]() {
        // 获取线程的 tid 值
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        func_(); // 开启一个新线程，专门执行该线程函数
    }));
    // 这里必须等待获取上面新线程的 tid,用信号量
    sem_wait(&sem);
}


void Thread::join() {
    joined_ = true;
    thread_->join();
}

// 给线程设定默认名称
void Thread::setDefaultName() {
    int num = ++numCreated_;
    if (name_.empty()) { // 线程没有名字
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}

