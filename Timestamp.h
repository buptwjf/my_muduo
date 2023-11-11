#pragma once
#include <iostream>
#include <string>

// 时间类
class Timestamp {
  public:
    Timestamp();
    Timestamp(int64_t microSecondsSinceEpoch);

    // 获取当前的时间
    static Timestamp now();

    // 将时间转化为 string 类型
    std::string toString() const;

    static const int kMicroSecondsPersecond = 1000 * 1000;

  private:
    int64_t microSecondsSinceEpoch_;
};
