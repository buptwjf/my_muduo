#pragma once // 编译器级别


/*
    noncopyable 的派生类不能拷贝构造和赋值，但是可以构造和析构
*/

class noncopyable {
  public:
    // 派生类不能拷贝构造和赋值
    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;

  protected: // 派生类可以访问，派生类可以构造和析构
    noncopyable() = default;
    ~noncopyable() = default;
};
