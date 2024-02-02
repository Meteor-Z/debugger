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

#include <bits/types/siginfo_t.h>
#include <sched.h>
#include <cstdint>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector>
#include "libelfin/dwarf/dwarf++.hh"
#include "libelfin/elf//elf++.hh"
#include "debugger/register.h"
#include "debugger/break_point.h"

namespace my_gdb {

enum class SymbolType {
    NOType,
    Object,  ///< 数据
    Func,    ///< 函数
    Section, ///<
    File,
};

struct Symbol {
    SymbolType type;
    std::string name;
    std::uintptr_t addr;
};

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

    /// TODO: 这里应该也可以直接直接 private
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

    /**
     * @brief 得到 pc 寄存器的数值
     *
     * @return uint64_t pc寄存器的数值
     */
    uint64_t get_pc_register();

    /**
     * @brief 设置 pc register 的数值
     *
     * @param value 数值
     */
    void set_pc_register(uint64_t value);

    /**
     * @brief 跳过断点
     *
     */
    void step_over_breakpoint();

    void wait_for_signal();

    /**
     * @brief 得到子进程发送的信息
     *
     * @return siginfo_t
     */
    siginfo_t get_signal_info();

    /// TODO: 这里可以进行一定的预处理，处理一遍，然后O（1）拿
    dwarf::die get_function_from_pc_register(uint64_t pc);

    dwarf::line_table::iterator get_line_entry_from_pc(uint64_t pc);

    /**
     * @brief 加载 基地址
     *
     */
    void init_load_address();

    /**
     * @brief 进行一定的偏移
     *
     * @param addr 虚拟地址
     * @return uint64_t 基地址
     */
    uint64_t offset_load_address(uint64_t addr);

    /**
     * @brief 打印源代码
     *
     * @param file_name
     * @param line
     * @param lines_context_number
     */
    void print_source(const std::string& file_name, unsigned line, unsigned lines_context_number = 2);

    /**
     * @brief 处理信息
     *
     * @param sign_info
     */
    void handle_sigtrap(siginfo_t sign_info);

    /**
     * @brief 单步执行
     *
     */
    void signle_step_instruction();

    void signle_step_instruction_with_breakpoint_check();

    void step_in();

    void step_out();

    /**
     * @brief 移除
     *
     * @param addr
     */
    void remove_breakpoints(std::intptr_t addr);

    uint64_t get_offset_pc();

    uint64_t offset_dwarf_address(uint64_t addr);

    /**
     * @brief 在函数的起点设置断点
     *
     * @param name
     */
    void set_breakpoint_at_function(const std::string& name);

    void set_breakpoint_at_souce_line(const std::string& file, unsigned line);

    std::vector<Symbol> lookup_symbol(const std::string& name);

    /**
     * @brief 打印调用栈
     *
     */
    void print_backtrace();

private:
    std::string m_program_name {};                                ///< 调试项目的名称
    pid_t m_pid { 0 };                                            ///< 调试项目的进程号
    std::unordered_map<std::intptr_t, BreakPoint> m_break_points; ///< 断点集合 <k, v>: 地址，和断点
    dwarf::dwarf m_dwarf {};                                      ///< dwarf
    elf::elf m_elf {};                                            ///< elf文件
    uint64_t m_load_address {};                                   ///< 偏移的地址
};
} // namespace my_gdb

#endif