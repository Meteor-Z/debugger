#pragma once

#ifndef MY_GDB_DEBUGGER_PTRACE_EXPR_CONTEXT_H
#define MY_GDB_DEBUGGER_PTRACE_EXPR_CONTEXT_H

#include <libelfin/dwarf/dwarf++.hh>

namespace my_gdb {
class PtraceExprContext : public dwarf::expr_context {
public:
    PtraceExprContext(pid_t pid);
    dwarf::taddr reg(unsigned reg_num) override;
    dwarf::taddr pc() override;
    dwarf::taddr deref_size(dwarf::taddr address, unsigned size) override;
private:
    pid_t m_pid;
};

} // namespace my_gdb

#endif