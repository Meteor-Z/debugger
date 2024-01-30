#include <bits/types/siginfo_t.h>
#include <cstring>
#include <fcntl.h>
#include <iterator>
#include <sys/user.h>
#include "fmt/base.h"
#include "sys/ptrace.h"
#include <cstdint>
#include <sys/wait.h>
#include <algorithm>
#include <functional>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include "fmt/format.h"
#include "libelfin/dwarf/dwarf++.hh"
#include "libelfin/elf/data.hh"
#include "libelfin/elf/elf++.hh"
#include "linenoise/linenoise.h"
#include "common/log.h"
#include "debugger/debugger.h"
#include "debugger/break_point.h"
#include "debugger/register.h"

namespace my_gdb {
Debugger::Debugger(const std::string& program_name, pid_t pid) : m_program_name(program_name), m_pid(pid) {
    int fd = open(m_program_name.c_str(), O_RDONLY);

    m_elf = elf::elf { elf::create_mmap_loader(fd) };
    m_dwarf = dwarf::dwarf { dwarf::elf::create_loader(m_elf) };
}

void Debugger::run() {
    wait_for_signal();
    init_load_address();
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
        DEBUG_LOG("from gdb: continue");
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
    auto it = std::find_if(begin(g_register_descriptors), end(g_register_descriptors),
                           [r](RegisterDescriptor rd) { return rd.r == r; });
    //    将其按照数据强转到 uint64_t 然后加上这个偏移量 就是数值了
    // user_regs_struct 是标准的POS类型
    return *(reinterpret_cast<uint64_t*>(&regs) + (it - begin(g_register_descriptors)));
    /// TODO: 设置一下，改成switch一样的
}

// void Debugger::set_register_value(pid_t pid, reg r, uint64_t value) {

//     struct user_regs_struct regs {};
//     ptrace(PTRACE_GETREGS, pid, nullptr, &regs);

//     auto item = std::find_if(std::begin(g_register_descriptors), end(g_register_descriptors),
//                              [r](RegisterDescriptor& rd) { return rd.r == r; });
//     /// TODO: 设置一下，改成switch一样的
//     *(reinterpret_cast<uint64_t*>(&regs) + (item - begin(g_register_descriptors))) = value;
//     // 设置上数值
//     ptrace(PTRACE_SETREGS, pid, nullptr, &regs);
// }
void Debugger::set_register_value(pid_t pid, reg r, uint64_t value) {
    user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, nullptr, &regs);
    auto it = std::find_if(begin(g_register_descriptors), end(g_register_descriptors),
                           [r](RegisterDescriptor rd) -> bool { return rd.r == r; });

    *(reinterpret_cast<uint64_t*>(&regs) + (it - begin(g_register_descriptors))) = value;
    ptrace(PTRACE_SETREGS, pid, nullptr, &regs);
}

uint64_t Debugger::get_register_value_from_dwarf_register(pid_t pid, unsigned reg_num) {
    auto item = std::find_if(begin(g_register_descriptors), end(g_register_descriptors),
                             [reg_num](RegisterDescriptor rd) -> bool { return rd.dwarf_r == reg_num; });
    if (item == std::end(g_register_descriptors)) {
        /// TODO: 错误处理
    }

    return get_register_value(pid, item->r);
}

std::string Debugger::get_register_name(reg r) {
    auto item = std::find_if(begin(g_register_descriptors), end(g_register_descriptors),
                             [r](RegisterDescriptor rd) -> bool { return rd.r == r; });
    return item->name;
}

reg Debugger::get_register_from_name(const std::string& name) {
    auto item = std::find_if(std::begin(g_register_descriptors), std::end(g_register_descriptors),
                             [name](auto&& rd) { return rd.name == name; });

    return item->r;
}

void Debugger::dump_all_registers_values() {
    for (const auto& register_descriptor : g_register_descriptors) {
        // 数据格式： 寄存器name : 数值
        std::cout << register_descriptor.name << " 0x" << std::setfill('0') << std::setw(16) << std::hex
                  << get_register_value(m_pid, register_descriptor.r) << std::endl;
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
    if (m_break_points.count(get_pc_register())) {
        auto& bp = m_break_points[get_pc_register()];
        if (bp.is_enable()) {
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

    auto sign_info = get_signal_info();

    if (sign_info.si_signo == SIGTRAP) {
        handle_sigtrap(sign_info);
    } else if (sign_info.si_signo == SIGSEGV) {
        std::cout << "Segmentation Fault:" << sign_info.si_code << std::endl;
    } else {
        DEBUG_LOG(fmt::format("get sign_info = {}", strsignal(sign_info.si_signo)));
    }
}

siginfo_t Debugger::get_signal_info() {
    siginfo_t info;
    ptrace(PTRACE_GETSIGINFO, m_pid, nullptr, &info);
    return info;
}
dwarf::die Debugger::get_function_from_pc_register(uint64_t pc) {
    // 全部开扫
    for (auto& cu : m_dwarf.compilation_units()) {
        // 如果包含 pc， 然后找这个tag,是否是包含 这个 pc
        if (dwarf::die_pc_range(cu.root()).contains(pc)) {
            for (const auto& die : cu.root()) {
                if (die.tag == dwarf::DW_TAG::subprogram) {
                    if (dwarf::die_pc_range(die).contains(pc)) {
                        return die;
                    }
                }
            }
        }
    }
    ERROR_LOG("can not find function");
    exit(0);
}

dwarf::line_table::iterator Debugger::get_line_entry_from_pc(uint64_t pc) {
    for (auto& cu : m_dwarf.compilation_units()) {
        if (dwarf::die_pc_range(cu.root()).contains(pc)) {
            auto& lt = cu.get_line_table();
            auto it = lt.find_address(pc);
            if (it == lt.end()) {
                ERROR_LOG("can not find line entry");
                exit(-1);
            } else {
                return it;
            }
        }
    }
    ERROR_LOG("can not find line entry");
    exit(-1);
}

void Debugger::init_load_address() {
    if (m_elf.get_hdr().type == elf::et::dyn) {
        std::ifstream map("/proc/" + std::to_string(m_pid) + "/maps");

        std::string addr;
        std::getline(map, addr, '-');
        DEBUG_LOG(fmt::format("addr = {}", addr));
        m_load_address = std::stol(addr, 0, 16);
    }
}

uint64_t Debugger::offset_load_address(uint64_t addr) { return addr - m_load_address; }

void Debugger::print_source(const std::string& file_name, unsigned line, unsigned lines_context_number /* = 2*/) {
    //    文件写操作
    std::ifstream file { file_name };

    auto start_line = line <= lines_context_number ? 1 : line - lines_context_number;
    auto end_line = line + lines_context_number + (line < lines_context_number ? lines_context_number - line : 0) + 1;
    char c {};
    auto current_line = 1u;
    while (current_line != start_line && file.get(c)) {
        if (c == '\n') {
            current_line++;
        }
    }

    std::cout << (current_line == line ? ">" : " ");

    while ((current_line <= end_line) && file.get(c)) {
        std::cout << c;
        if (c == '\n') {
            current_line++;
            std::cout << (current_line == line ? "> " : " ");
        }
    }
    std::cout << std::endl;
}

void Debugger::handle_sigtrap(siginfo_t sign_info) {
    if (sign_info.si_code == SI_KERNEL || sign_info.si_code == TRAP_BRKPT) {
        set_pc_register(get_pc_register() - 1);
        std::cout << "hit breakpoint at address 0x" << std::hex << get_pc_register() << std::endl;
        auto offest_pc = offset_load_address(get_pc_register());
        auto line_entry = get_line_entry_from_pc(offest_pc);
        print_source(line_entry->file->path, line_entry->line);
        return;
    } else if (sign_info.si_code == TRAP_TRACE) {
        return;
    } else {
        std::cout << "Unknown SIGTRACP code :" << sign_info.si_code << std::endl;
        return;
    }
}
} // namespace my_gdb