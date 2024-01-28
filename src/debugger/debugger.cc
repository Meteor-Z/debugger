#include <cstdint>
#include <functional>
#include <ios>
#include <sys/wait.h>
#include "debugger/break_point.h"
#include "fmt/base.h"
#include "sys/ptrace.h"
#include <algorithm>
#include <iterator>
#include <sstream>
#include <typeinfo>
#include <vector>
#include <string>
#include "debugger/debugger.h"
#include "linenoise/linenoise.h"
#include "common/log.h"
#include <iostream>

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
    ptrace(PTRACE_CONT, m_pid, nullptr, nullptr);
    int wait_status;
    auto options { 0 };
    waitpid(m_pid, &wait_status, options);
}

void Debugger::set_breakpoint_at_address(std::intptr_t addr) {
    std::cout << "set breakpoint at 0x " << std::hex << addr << std::endl;
    BreakPoint break_point { m_pid, addr };
    break_point.enable();
    m_break_points[addr] = break_point;
}
} // namespace my_gdb