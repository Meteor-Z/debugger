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
#include "debugger/register.h"
#include <cstdint>
#include <sched.h>
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

    ///TODO: 这里应该也可以直接直接 private
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

    /**
     * @brief 得到寄存器的数值
     *
     * @param pid 进程号
     * @param r 得到的寄存器
     * @return uint64_t 寄存器上的数值
     */
    uint64_t get_register_value(pid_t pid, reg r);

    /**
     * @brief 设置寄存器上的数值
     *
     * @param pid 进程号
     * @param r 寄存器
     * @param value 数值
     */
    void set_register_value(pid_t pid, reg r, uint64_t value);

    /**
     * @brief 从 dwarf 编号上得到 寄存器的数据
     *
     * @param pid 进程号
     * @param reg_num  寄存器的 dwarf 编号
     * @return uint64_t 数据
     */
    uint64_t get_register_value_from_dwarf_register(pid_t pid, unsigned reg_num);

    /**
     * @brief 得到寄存器的信息
     * 
     * @param r 哪一个寄存器
     * @return std::string 信息
     */
    std::string get_register_name(reg r);

    reg get_register_from_name(const std::string& name);

    /**
     * @brief 打印出所有寄存器的数据
     * 
     */
    void dump_all_registers_values();

    /**
     * @brief 读某一地址上的数值
     * 
     * @param address 地址
     * @return uint64_t 数值
     */
    uint64_t read_memory(uint64_t address);

    /**
     * @brief 写入数值
     * 
     * @param address 地址
     * @param value 数值
     */
    void write_memory(uint64_t address, uint64_t value);

private:
    std::string m_program_name {};                                ///< 调试项目的名称
    pid_t m_pid { 0 };                                            ///< 调试项目的进程号
    std::unordered_map<std::intptr_t, BreakPoint> m_break_points; ///< 断点集合 <k, v>: 地址，和断点
};
} // namespace my_gdb

#endif