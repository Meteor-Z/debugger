/**
 * @file debugger.h
 * @author lzc (liuzechen.coder@qq.com)
 * @brief 调试器
 * @version 0.1
 * @date 2024-01-28
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef MY_GDB_DEBUGGER_DEBUGGER_H
#define MY_GDB_DEBUGGER_DEBUGGER_H

#include "debugger/break_point.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace my_gdb {
class Debugger {
public:
    /**
     * @brief Construct a new Debugger object
     *
     * @param program_name 程序名称
     * @param pid 进程号
     */
    Debugger(const std::string& program_name, pid_t pid);

    /**
     * @brief 启动debug调试
     *
     */
    void run();

    /**
     * @brief 命令处理
     *
     * @param line
     */
    void handle_command(const std::string& line);

private:
    /**
     * @brief 按照 delimiter进行切分
     *
     * @param line 总命令
     * @param delimiter 按照什么切分
     * @return std::vector<std::string> 切分出来的命令
     */
    std::vector<std::string> spilt(const std::string& line, char delimiter);

    /**
     * @brief 判断 一个字符串是否是另一个字符串的前缀
     *
     * @param line 主字符串
     * @param of 包含的字符串
     * @return true of 是 line 的前缀
     * @return false of 不是 line 的前缀
     */
    bool is_prefix(const std::string& line, const std::string& of);

    /**
     * @brief continue命令的时候执行
     *
     */
    void continue_execution();

    /**
     * @brief 在这个地址上设置断点
     * 
     */
    void set_breakpoint_at_address(std::intptr_t addr);

private:
    std::string m_program_name {}; ///< 调试项目的名称
    pid_t m_pid { 0 };             ///< 调试项目的进程号
    std::unordered_map<std::intptr_t, BreakPoint> m_break_points; ///< 断点集合 <k, v>: 地址，和断点
};
} // namespace my_gdb

#endif