/**
 * @file break_point.h
 * @author lzc (liuzechen.coder@qq.com)
 * @brief 断点
 * @version 0.1
 * @date 2024-01-28
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <cstdint>
#include <fcntl.h>

#ifndef MY_GDB_DEBUGGER_BREAK_POINT_H
#define MY_GDB_DEBUGGER_BREAK_POINT_H

namespace my_gdb {
class BreakPoint {
public:
    BreakPoint() = default;
    BreakPoint(pid_t pid, std::intptr_t addr);

    /**
     * @brief 设置断点
     * @note 0xcc（INT 3) 设置断点，取出当前函数中的数据，然后最后一位填上0xcc,最后还要保留那个数据，之后再补充上
     */
    void enable();

    /**
     * @brief 取消断点
     * @note 更简单，就是将原来的地址填上原来的数据就行了
     */
    void disable();

    bool is_enable();
    std::intptr_t get_address();

private:
    pid_t m_pid;              ///< 调试的进程号
    std::intptr_t m_addr;     ///< 打断点的地址
    bool m_enabled { false }; ///< 是否打上断点了
    uint8_t m_saved_data {};  ///< 位于断点上的数据
};

} // namespace my_gdb

#endif