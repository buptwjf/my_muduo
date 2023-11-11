//
// Created by 86188 on 2023/9/5.
//
#pragma once

#include <vector>
#include <strings.h>
#include <string>
#include <algorithm>
#include <assert.h>

/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
class Buffer {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitalSize = 1024;

    explicit Buffer(size_t initialsize = kInitalSize) :
            buffer_(kCheapPrepend + initialsize), // buffer 底层的长度 8+1024
            readerIndex_(kCheapPrepend), // 8
            writerIndex_(kCheapPrepend) { // 8
    }


    void swap(Buffer &rhs) {
        buffer_.swap(rhs.buffer_);
        std::swap(readerIndex_, rhs.readerIndex_);
        std::swap(writerIndex_, rhs.writerIndex_);
    }

    // 可读数据的长度
    size_t readableBytes() const {
        return writerIndex_ - readerIndex_;
    }

    // 可写的缓冲区的长度
    size_t writeableBytes() const {
        return buffer_.size() - writerIndex_;
    }

    size_t prependableBytes() const {
        return readerIndex_;
    }

    // 返回 可读数据缓冲区 的起始地址
    const char *peek() const {
        return begin() + readerIndex_;
    };

    // onMessage string <- Buffer
    void retrieve(size_t len) {
        if (len < readableBytes()) {
            readerIndex_ += len;  // 应用只读取了缓冲区的一部分，就是 len 长度，还剩下
            // readerIndex_ += len ~ writeIndex 的数据
        } else {
            retrieveAll();
        }
    }

    void retrieveAll() {
        readerIndex_ = writerIndex_ = kCheapPrepend;
    }

    // 把 onMessage 函数上报的 Buffer 数据，转成 string 类型的数据返回
    std::string retrieveAllAsString() {
        return retrieveAsString(readableBytes()); // 应用可读取的数据长度
    }


    std::string retrieveAsString(size_t len) {
        assert(len <= readableBytes());
        std::string result(peek(), len);  // 构造 result
        retrieve(len); // 上面把缓冲区可读的数据，已经读出来了，因此要对缓冲区进行复位操作
        return result;
    }

    // buffer_.size - writeIndex_  len
    // 判断可写缓冲区是否有 len 的长度
    void ensureWriteableBytes(size_t len) {
        if (writeableBytes() < len) {
            makeSpace(len); // 扩容函数
        }
    }

    // 把 [data, data+len] 内存上的数据，添加到 writeable 缓冲区上
    void append(const char *data, size_t len) {
        ensureWriteableBytes(len);
        std::copy(data, data + len, beginWrite());
        writerIndex_ += len;
    }

    char *beginWrite() {
        return begin() + writerIndex_;
    }

    const char *beginWrite() const {
        return begin() + writerIndex_;
    }

    // 从 fd 上读取数据
    ssize_t readFd(int fd, int *saveErrno);

private:

    // 扩容函数
    void makeSpace(size_t len) {
        if (writeableBytes() + prependableBytes() < len + kCheapPrepend) { // 前一部分空闲的 + 可写的 小于 要求的大小
            buffer_.resize((writerIndex_ + len));
        } else {
            size_t readable = readableBytes();
            // read 中未读的数据 向前移动
            std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

    // 返回 buffer_ 底层 vector 首个元素的地址，也就是数组的起始地址
    char *begin() {
        return &*buffer_.begin();
    }

    const char *begin() const {
        return &*buffer_.begin();
    }


    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
    static const char kCRLF[];
};