#pragma once

#ifndef MY_GDB_DEBUGGER_REGISTER_H
#define MY_GDB_DEBUGGER_REGISTER_H

#include <cstddef>
#include <string>
#include <array>

namespace my_gdb {
    
/**
 * @brief 各个寄存器
 *
 */
enum class reg {
    rax,
    rbx,
    rcx,
    rdx,
    rdi,
    rsi,
    rbp,
    rsp,
    r8,
    r9,
    r10,
    r11,
    r12,
    r13,
    r14,
    r15,
    rip,
    rflags,
    cs,
    orig_rax,
    fs_base,
    gs_base,
    fs,
    gs,
    ss,
    ds,
    es
};
constexpr size_t registers_number { 27 }; ///< 27 个寄存器

/**
 * @brief 寄存器描述符
 *
 */
struct RegisterDescriptor {
    reg r;            ///< 哪一个寄存器
    int dwarf_r;      ///< 寄存器 dwarf 编号
    std::string name; ///< 名称
};

/**
 * @brief 描述符的对应关系
 * @note 根据此struct user_regs_struct 生成
 */
inline const std::array<RegisterDescriptor, registers_number> g_register_descriptors { {
    { reg::r15, 15, "r15" },
    { reg::r14, 14, "r14" },
    { reg::r13, 13, "r13" },
    { reg::r12, 12, "r12" },
    { reg::rbp, 6, "rbp" },
    { reg::rbx, 3, "rbx" },
    { reg::r11, 11, "r11" },
    { reg::r10, 10, "r10" },
    { reg::r9, 9, "r9" },
    { reg::r8, 8, "r8" },
    { reg::rax, 0, "rax" },
    { reg::rcx, 2, "rcx" },
    { reg::rdx, 1, "rdx" },
    { reg::rsi, 4, "rsi" },
    { reg::rdi, 5, "rdi" },
    { reg::orig_rax, -1, "orig_rax" },
    { reg::rip, -1, "rip" },
    { reg::cs, 51, "cs" },
    { reg::rflags, 49, "eflags" },
    { reg::rsp, 7, "rsp" },
    { reg::ss, 52, "ss" },
    { reg::fs_base, 58, "fs_base" },
    { reg::gs_base, 59, "gs_base" },
    { reg::ds, 53, "ds" },
    { reg::es, 50, "es" },
    { reg::fs, 54, "fs" },
    { reg::gs, 55, "gs" },
} };

} // namespace my_gdb
#endif