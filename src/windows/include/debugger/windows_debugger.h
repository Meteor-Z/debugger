#pragma once

#if defined(_WIN32)

#include <windows.h>
#include <wingdi.h>
#include <winnt.h>
#include <minwinbase.h>

#endif

namespace debugger {
class WindowsDebugger {
public:
    WindowsDebugger() = delete;
    WindowsDebugger(const WindowsDebugger&) = delete;
    WindowsDebugger(WindowsDebugger&&) = delete;
    WindowsDebugger& operator=(const WindowsDebugger&) = delete;
    WindowsDebugger& operator=(WindowsDebugger&&) = delete;

    WindowsDebugger(const HANDLE& process);

    void OnProcessCreated(const CREATE_PROCESS_DEBUG_INFO*);
    void OnThreadCreated(const CREATE_THREAD_DEBUG_INFO*);
    void OnException(const EXCEPTION_DEBUG_INFO*);
    void OnProcessExited(const EXIT_PROCESS_DEBUG_INFO*);
    void OnThreadExited(const EXIT_THREAD_DEBUG_INFO*);
    void OnOutputDebugString(const OUTPUT_DEBUG_STRING_INFO* info);
    void OnRipEvent(const RIP_INFO*);
    void OnDllLoaded(const LOAD_DLL_DEBUG_INFO*);
    void OnDllUnloaded(const UNLOAD_DLL_DEBUG_INFO*);

private:
    HANDLE m_process{}; ///< 调试进程的 process
};
} // namespace debugger