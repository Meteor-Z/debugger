#include "debugger/ptrace_expr_context.h"
#include "debugger/debugger.h"
#include <sys/ptrace.h>
#include <sys/user.h>
namespace my_gdb {

PtraceExprContext::PtraceExprContext(pid_t pid) : m_pid(pid) {}
dwarf::taddr PtraceExprContext::reg(unsigned reg_num) {
    return my_gdb::Debugger::get_register_value_from_dwarf_register(m_pid, reg_num);
}
dwarf::taddr PtraceExprContext::pc() {
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, m_pid, nullptr, &regs);
    return regs.rip;
}

dwarf::taddr PtraceExprContext::deref_size(dwarf::taddr address, unsigned size) {
    return ptrace(PTRACE_PEEKDATA, m_pid, address, nullptr);
}

} // namespace my_gdb