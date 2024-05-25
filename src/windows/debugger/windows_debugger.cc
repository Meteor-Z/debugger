#include "../include/debugger/windows_debugger.h"

namespace debugger {

WindowsDebugger::WindowsDebugger(const HANDLE& process) : m_process(process) {}

void WindowsDebugger::OnProcessCreated(const CREATE_PROCESS_DEBUG_INFO*) {}
void WindowsDebugger::OnThreadCreated(const CREATE_THREAD_DEBUG_INFO*) {}
void WindowsDebugger::OnException(const EXCEPTION_DEBUG_INFO*) {}
void WindowsDebugger::OnProcessExited(const EXIT_PROCESS_DEBUG_INFO*) {}
void WindowsDebugger::OnThreadExited(const EXIT_THREAD_DEBUG_INFO*) {}
void WindowsDebugger::OnOutputDebugString(const OUTPUT_DEBUG_STRING_INFO* info) {}
void WindowsDebugger::OnRipEvent(const RIP_INFO*) {}
void WindowsDebugger::OnDllLoaded(const LOAD_DLL_DEBUG_INFO*) {}
void WindowsDebugger::OnDllUnloaded(const UNLOAD_DLL_DEBUG_INFO*) {}
} // namespace debugger