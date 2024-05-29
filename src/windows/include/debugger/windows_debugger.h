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

    /**
     * @brief 创建项目进程的时候触发的断点
     *
     * @param info
     */
    void OnProcessCreated(const CREATE_PROCESS_DEBUG_INFO* info);

    /**
     * @brief 进程创建的时候
     *
     * @param info
     */
    void OnThreadCreated(const CREATE_THREAD_DEBUG_INFO* info);

    /**
     * @brief Windows上的异常还分一次两次，有两次处理的机会？
     * 好怪，这里还要研究一下
     *
     * @param info
     */
    void OnException(const EXCEPTION_DEBUG_INFO* info);

    /**
     * @brief 进程退出的时候
     *
     * @param info
     */
    void OnProcessExited(const EXIT_PROCESS_DEBUG_INFO& info);

    /**
     * @brief 线程退出的时候
     *
     * @param info
     */
    void OnThreadExited(const EXIT_THREAD_DEBUG_INFO& info);
    /**
     * @brief 调用 Output Debug String 的时候就会触发这类事
     * 然后输出相关相关调试信息
     *
     * @param info
     */
    void on_output_debug_string(const OUTPUT_DEBUG_STRING_INFO* info);
    void OnRipEvent(const RIP_INFO& info);
    void OnDllLoaded(const LOAD_DLL_DEBUG_INFO& info);
    void OnDllUnloaded(const UNLOAD_DLL_DEBUG_INFO& info);

public:
    static inline DWORD g_continue_status{
        DBG_EXCEPTION_NOT_HANDLED}; ///< 当前调试的进度，因为调试的时候是分两次的
private:
    HANDLE m_process{}; ///< 调试进程的 process
};
} // namespace debugger