#include <iterator>
#include <sys/user.h>
#include "fmt/format.h"
#include "sys/ptrace.h"
#include <sys/wait.h>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <type_traits>
#include "linenoise/linenoise.h"
#include "common/log.h"
#include "debugger/debugger.h"
#include "debugger/break_point.h"
#include "debugger/register.h"

namespace my_gdb {
Debugger::Debugger(const std::string& program_name, pid_t pid) : m_program_name(program_name), m_pid(pid) {}

void Debugger::run() {
    int wait_status;
    int options { 0 };
    // pid_t waitpid(pid_t pid,int *status,int options);
    // pid: 调试的进程号, status: 状态码，options:信息？
    waitpid(m_pid, &wait_status, options);
    char* line { nullptr };

    while ((line = linenoise("my_gdb> ")) != nullptr) {
        DEBUG_LOG(fmt::format("line = {}", line));
        handle_command(line);
        // 加上历史信息
        linenoiseHistoryAdd(line);
        // 释放指向的内存
        linenoiseFree(line);
    }
}

/// TODO: 有点乱，可以再封装一下
void Debugger::handle_command(const std::string& line) {
    if (line.empty()) {
        return;
    }
    std::vector<std::string> args = spilt(line, ' ');
    // fmt::print("args = {}", args);
    std::string command = args[0];

    if (is_prefix(command, "continue")) {
        std::cout << "yes" << std::endl;
        continue_execution();
    } else if (is_prefix(command, "break")) {
        // 0x开头，那么就是后面的
        std::string addr { args[1], 2 };
        // 从0位开始，然后 base 是 16
        set_breakpoint_at_address(std::stol(addr, 0, 16));
    } else if (is_prefix(command, "register")) {
        if (is_prefix(args[1], "dump")) {
            dump_all_registers_values();
        } else if (is_prefix(args[1], "read")) {
            std::cout << get_register_value(m_pid, get_register_from_name(args[2])) << std::endl;
        } else if (is_prefix(args[1], "write")) {
            std::string val { args[3], 2 }; // 0xval
            set_register_value(m_pid, get_register_from_name(val), stol(val, 0, 16));
        }
    } else if (is_prefix(command, "memory")) {
        std::string addr { args[2], 2 };
        if (is_prefix(args[1], "read")) {
            std::cout << std::hex << read_memory(std::stol(addr, 0, 16)) << std::endl;
        }
        if (is_prefix(args[1], "write")) {
            std::string val { args[3], 2 }; // assume 0xVAL
            write_memory(std::stol(addr, 0, 16), std::stol(val, 0, 16));
        }

    } else {
        std::cerr << "unknown command" << std::endl;
    }
}

std::vector<std::string> Debugger::spilt(const std::string& line, char delimiter) {
    std::vector<std::string> out {};
    std::stringstream ss { line };
    std::string result {};
    while (std::getline(ss, result, delimiter)) {
        out.push_back(result);
    }

    return out;
}

bool Debugger::is_prefix(const std::string& line, const std::string& of) {
    if (line.size() > of.size()) {
        return false;
    }
    return std::equal(line.begin(), line.end(), of.begin());
}

void Debugger::continue_execution() {
    // 执行
    step_over_breakpoint();
    ptrace(PTRACE_CONT, m_pid, nullptr, nullptr);
    wait_for_signal();
}

void Debugger::set_breakpoint_at_address(std::intptr_t addr) {
    std::cout << "set breakpoint at 0x " << std::hex << addr << std::endl;
    BreakPoint break_point { m_pid, addr };
    break_point.enable();
    m_break_points[addr] = break_point;
}

uint64_t Debugger::get_register_value(pid_t pid, reg r) {
    struct user_regs_struct regs {};
    // GETREFS 得到所有寄存器的数值，赋值给 regs
    ptrace(PTRACE_GETREGS, pid, nullptr, &regs);
    auto it = std::find_if(std::begin(g_register_descriptors), std::end(g_register_descriptors),
                           [r](RegisterDescriptor&& rd) { return rd.m_reg == r; });
    //    将其按照数据强转到 uint64_t 然后加上这个偏移量 就是数值了
    // user_regs_struct 是标准的POS类型
    return *(reinterpret_cast<uint64_t*>(&regs) + (it - begin(g_register_descriptors)));
    /// TODO: 设置一下，改成switch一样的
}

void Debugger::set_register_value(pid_t pid, reg r, uint64_t value) {

    struct user_regs_struct regs {};
    ptrace(PTRACE_GETREGS, pid, nullptr, &regs);

    auto item = std::find_if(std::begin(g_register_descriptors), std::end(g_register_descriptors),
                             [r](RegisterDescriptor&& rd) { return rd.m_reg == r; });
    /// TODO: 设置一下，改成switch一样的
    *(reinterpret_cast<uint64_t*>(&regs) + (item - begin(g_register_descriptors))) = value;
    // 设置上数值
    ptrace(PTRACE_SETREGS, pid, nullptr, &regs);
}

uint64_t Debugger::get_register_value_from_dwarf_register(pid_t pid, unsigned reg_num) {
    using std::begin;
    auto item = std::find_if(std::begin(g_register_descriptors), std::end(g_register_descriptors),
                             [reg_num](RegisterDescriptor&& rd) { return rd.m_dwarf_r = reg_num; });
    if (item == std::end(g_register_descriptors)) {
        /// TODO: 错误处理
    }

    return get_register_value(pid, item->m_reg);
}

std::string Debugger::get_register_name(reg r) {
    auto item = std::find_if(std::begin(g_register_descriptors), std::end(g_register_descriptors),
                             [r](RegisterDescriptor&& rd) { return rd.m_reg = r; });
    return item->m_name;
}

reg Debugger::get_register_from_name(const std::string& name) {
    auto item = std::find_if(std::begin(g_register_descriptors), std::end(g_register_descriptors),
                             [name](RegisterDescriptor&& rd) { return rd.m_name == name; });

    return item->m_reg;
}

void Debugger::dump_all_registers_values() {
    for (const auto& register_descriptor : g_register_descriptors) {
        // 数据格式： 寄存器name : 数值
        std::cout << register_descriptor.m_name << " 0x" << std::setfill('0') << std::setw(16) << std::hex
                  << get_register_value(m_pid, register_descriptor.m_reg) << std::endl;
    }
}

uint64_t Debugger::read_memory(uint64_t address) { return ptrace(PTRACE_PEEKDATA, m_pid, address, nullptr); }

void Debugger::write_memory(uint64_t address, uint64_t value) { ptrace(PTRACE_POKEDATA, m_pid, address, value); }

uint64_t Debugger::get_pc_register() { return get_register_value(m_pid, reg::rip); }
void Debugger::set_pc_register(uint64_t value) { set_register_value(m_pid, reg::rip, value); }

// 将执行放回断点之前，然后禁用他，然后单步执行，然后再启动他
void Debugger::step_over_breakpoint() {

    // 当PC寄存器会存放下一个地址在哪，这时候PC = PC + 取出来的长度，
    // 上一条指令就是 pc - 1, 然后取出来
    /// TODO: 这里好像有不严谨的地方
    auto possile_breakpoint_location = get_pc_register() - 1;
    if (m_break_points.count(possile_breakpoint_location)) {
        auto& bp = m_break_points[possile_breakpoint_location];
        if (bp.is_enable()) {
            auto previous_instruction_address = possile_breakpoint_location;

            set_pc_register(previous_instruction_address);

            bp.disable();

            ptrace(PTRACE_SINGLESTEP, m_pid, nullptr, nullptr);

            wait_for_signal();

            bp.enable();
        }
    }
}
void Debugger::wait_for_signal() {
    int wait_status;
    auto options = 0;
    waitpid(m_pid, &wait_status, options);
}
} // namespace my_gdb