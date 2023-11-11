#include "InetAddress.h"
#include "strings.h" // ?
#include <string.h>
// 打包 ip 地址和端口号
InetAddress::InetAddress(uint16_t port, std::string ip) {
    bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr =
        inet_addr(ip.c_str()); // 把字符串转成整数的点分十进制，以及网络字节序
}

// 输出 ip 地址
std::string InetAddress::toIp() const {
    // addr_
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf); //
    return buf;
}

// 输出 ip 和端口号
std::string InetAddress::toIpPort() const {
    // ip:
    char buf[64] = {0};
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf); //
    size_t end = strlen(buf);
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf + end, ":%u", port);
    return buf;
}

// 输出端口号
uint16_t InetAddress::toPort() const { return ntohs(addr_.sin_port); }

#include <iostream>
int main() {
    InetAddress addr(8080);
    // addr.getSockAddr();
    std::cout << addr.toIp() << std::endl;
    std::cout << addr.toIpPort() << std::endl;
    std::cout << addr.toPort() << std::endl;
    return 0;
}