//
// Created by 86188 on 2023/8/30.
//

#include "CurrentThread.h"

namespace CurrentThread {
    __thread int t_cachedTid = 0;

    void cachedTid() {
        if (t_cachedTid == 0) {
            // 通过系统调用获得当前线程的 tid 值
            t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }
};