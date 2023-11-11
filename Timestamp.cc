#include "Timestamp.h"
//#include <sys/time.h>

Timestamp::Timestamp() : microSecondsSinceEpoch_(0) {}

// 有参构造函数指明不支持隐式转换
Timestamp::Timestamp(int64_t microSecondsSinceEpoch)
        : microSecondsSinceEpoch_(microSecondsSinceEpoch) {}

// 获取当前的时间
Timestamp Timestamp::now() {
    // 陈硕原版
    // struct timeval tv;
    // gettimeofday(&tv, NULL);
    // int64_t seconds = tv.tv_sec;
    // return Timestamp(seconds * kMicroSecondsPersecond + tv.tv_usec);

    return Timestamp(time(NULL));
}

// 将时间转化为 string 类型
std::string Timestamp::toString() const {
    // 这里与陈硕不同
    char buf[128] = {0};
    tm *tm_time = localtime(&microSecondsSinceEpoch_);

    snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d", tm_time->tm_year + 1900,
             tm_time->tm_mon + 1, tm_time->tm_mday, tm_time->tm_hour,
             tm_time->tm_min, tm_time->tm_sec);
    return buf;
}

// #ifdef 0
// // 测试时间输出
#include <iostream>

//int main() { std::cout << Timestamp::now().toString() << std::endl; }
// #endif