#include "debugger/break_point.h"
#include <cstdint>
#include <sys/ptrace.h>

namespace my_gdb {

BreakPoint::BreakPoint(pid_t pid, std::intptr_t addr) : m_pid(pid), m_addr(addr), m_enabled(false), m_saved_data() {}

void BreakPoint::enable() {

    auto data = ptrace(PTRACE_PEEKDATA, m_pid, m_addr, nullptr);
    // uint_8 一个字节， 八位
    // 1111 1111 八位，保留底层的字节
    m_saved_data = static_cast<uint8_t>(data & 0xff);
    uint64_t int3 = 0xcc;
    uint64_t data_with_int3 = ((data & ~0xff) | int3);
    // 在当前地址上填上
    ptrace(PTRACE_POKEDATA, m_pid, m_addr, data_with_int3);

    m_enabled = true;
}
void BreakPoint::disable() {
    auto data = ptrace(PTRACE_PEEKDATA, m_pid, m_addr, nullptr);
    // 取前面的数据，然后 | 后面的数据
    auto result_data = ((data & ~0xff) | m_saved_data);
    ptrace(PTRACE_POKEDATA, m_pid, m_addr, result_data);

    m_enabled = false;
}

bool BreakPoint::is_enable() { return m_enabled; }
std::intptr_t BreakPoint::get_address() { return m_addr; }
} // namespace my_gdb
