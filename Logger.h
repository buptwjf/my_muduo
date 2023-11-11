#pragma once

#include <iostream>
#include <string>

// 日志相关的代码

// LG_INFO("%s %d", arg1, qrg2)
#define LOG_INFO(logmsgFormat, ...)                                            \
    do {                                                                       \
        Logger &logger = Logger::instance();                                   \
        logger.setLogLevel(INFO);                                              \
        char buf[1024] = {0};                                                  \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);                      \
        logger.log(buf);                                                       \
    } while (0)

#define LOG_ERROR(logmsgFormat, ...)                                           \
    do {                                                                       \
        Logger &logger = Logger::instance();                                   \
        logger.setLogLevel(INFO);                                              \
        char buf[1024] = {0};                                                  \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);                      \
        logger.log(buf);                                                       \
    } while (0)

#define LOG_FATAL(logmsgFormat, ...)                                           \
    do {                                                                       \
        Logger &logger = Logger::instance();                                   \
        logger.setLogLevel(INFO);                                              \
        char buf[1024] = {0};                                                  \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);                      \
        logger.log(buf);                                                       \
        exit(-1);                                                               \
    } while (0)


//#ifdef MUDEBUG
#define LOG_DEBUG(logmsgFormat, ...)                                               \
    do {                                                                       \
        Logger &logger = Logger::instance();                                   \
        logger.setLogLevel(INFO);                                              \
        char buf[1024] = {0};                                                  \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);                      \
        logger.log(buf);                                                       \
    } while (0)
//#endif

// 定义日志的级别 INFO ERROR FATAL DEBUG
enum LogLevel {
    INFO,  // 普通信息
    ERROR, // 错误信息
    FATAL, // core信息
    DEBUG, // 调试细腻些
};

// 输出日志类
class Logger {
public:
    // 获取日志的唯一对象
    static Logger &instance();

    // 定义日志级别
    void setLogLevel(int level);

    // 写日志
    void log(std::string msg);

private:
    int logLevel_;

    Logger();
};
