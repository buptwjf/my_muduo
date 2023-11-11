//
// Created by 86188 on 2023/9/5.
//

#include "Buffer.h"
#include <errno.h>
#include <sys/uio.h>

/*
 * 从 fd 上读取数据，Poller 工作在 LT 模式
 * Buffer 缓冲区是有大小的！但是从 fd 上读数据的时候，却不知道 tcp 数据最终的大小
 * 应该开多大的缓冲区？
 * */
ssize_t Buffer::readFd(int fd, int *saveErrno) {
    char extraBuf[65530] = {0}; // 64 k 的空间
    // readv 可以根据读出来的数据自动的填充多个缓冲区
    struct iovec vec[2];
    const size_t writable = writeableBytes(); // Buffer 底层缓冲区剩余的可读写缓冲区的大小
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;

    vec[1].iov_base = extraBuf;
    vec[1].iov_len = sizeof extraBuf;

    const int iovcnt = (writable < sizeof(extraBuf)) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt); // readv 和 writev

    if (n < 0) {
        *saveErrno = errno;
    } else if (n <= writable) { // Buffer 的可写缓冲区已经够存储读出来的数据
        writerIndex_ += n;
    } else { // extrabuf 里面也写入了数据，进行 buf 扩容
        writerIndex_ = buffer_.size();
        append(extraBuf, n - writable); // 从 writeIndex_ 开始写
    }
    return n;

}