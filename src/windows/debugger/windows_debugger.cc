#include "../include/debugger/windows_debugger.h"
#include <cstring>
#include <handleapi.h>
#include <iomanip>
#include <ios>
#include <iostream>
#include <memoryapi.h>
#include <minwinbase.h>
#include <minwindef.h>
#include <sstream>
#include <string>
#include <stringapiset.h>
#include <urlmon.h>
#include <vector>
#include <winnls.h>
#include <winnt.h>
#include <tchar.h>
#include <windows.h>

namespace debugger {

void WindowsDebugger::get_command(std::vector<std::string>& command) {
    std::string ans;
    std::getline(std::cin, ans);

    std::stringstream ssin(ans);
    std::string temp;

    while (ssin >> temp) {
        command.push_back(temp);
    }
}
WindowsDebugger::WindowsDebugger(const HANDLE& process, const HANDLE& thread,
    const DWORD& dw_process_id, const DWORD& dw_thread_id)
    : m_process(process), m_thread(thread), m_dw_process_id(dw_process_id),
      m_dw_thread_id(dw_thread_id) {}

void WindowsDebugger::OnProcessCreated(const CREATE_PROCESS_DEBUG_INFO* info) {
    CloseHandle(info->hFile);
    CloseHandle(info->hThread);
    CloseHandle(info->hProcess);

    std::cout << "debugger was created" << std::endl;
}
void WindowsDebugger::OnThreadCreated(const CREATE_THREAD_DEBUG_INFO* info) {
    CloseHandle(info->hThread);

    std::cout << "New Thread was created" << std::endl;
}
void WindowsDebugger::OnException(const EXCEPTION_DEBUG_INFO* info) {
    std::cout << "Exception occured" << std::endl;
    std::cout << "Exception Code:" << std::hex << std::uppercase << std::setw(8)
              << std::setfill('0') << info->ExceptionRecord.ExceptionCode
              << std::dec << std::endl;
    // 上面是打印的相关函数

    // 第一次处理
    if (info->dwFirstChance == true) {
        std::cout << "First Chance" << std::endl;
        g_continue_status = DBG_EXCEPTION_NOT_HANDLED;

    } else {
        std::cout << "Second Chance" << std::endl;
        g_continue_status = DBG_CONTINUE;
    }
}
void WindowsDebugger::OnProcessExited(const EXIT_PROCESS_DEBUG_INFO& info) {
    std::cout << "Debuger was terminted" << std::endl;
    std::cout << "Exit Code" << info.dwExitCode << std::endl;
}
void WindowsDebugger::OnThreadExited(const EXIT_THREAD_DEBUG_INFO& info) {
    std::cout << "Thread was terminated" << std::endl;
    std::cout << "Exit Code:" << info.dwExitCode << std::endl;
}
void WindowsDebugger::on_output_debug_string(
    const OUTPUT_DEBUG_STRING_INFO* info) {
    char* p_buffer = (char*)malloc(info->nDebugStringLength * sizeof(char));
    size_t bytes_read;

    // 从另一个地址空间中读取内存，给相关句柄，然后读取相关内存
    if (!ReadProcessMemory(m_process,
            info->lpDebugStringData,
            p_buffer,
            info->nDebugStringLength,
            &bytes_read)) {
        std::cerr << "寄了" << std::endl;
    }

    int require_len = info->nDebugStringLength;

    char* str_ans = (char*)malloc(require_len + 1);

    std::memcpy(str_ans, p_buffer, require_len);

    str_ans[require_len] = '\0';

    std::cout << "Debug string :" << ' ' << str_ans << std::endl;

    free(p_buffer);
    free(str_ans);

    g_continue_status = DBG_CONTINUE;
}
void WindowsDebugger::OnRipEvent(const RIP_INFO& info) {
    std::cout << "Rip Event occured" << std::endl;
}
void WindowsDebugger::OnDllLoaded(const LOAD_DLL_DEBUG_INFO& info) {
    CloseHandle(info.hFile);
    std::cout << "Dll was loaded" << std::endl;
}
void WindowsDebugger::OnDllUnloaded(const UNLOAD_DLL_DEBUG_INFO& info) {
    std::cout << "A dll was unloaded." << std::endl;
}

bool WindowsDebugger::start_debug(const std::string& command) {
    if (command.size() < 2) {
        return false;
    } else {
    }
}
void WindowsDebugger::on_continue(const std::vector<std::string>& command) {
    if (command.size() < 2) {
        handle_excetion(false);
        return;
    }

    if (command[1] == "c") {
        handle_excetion(true);
        continue_debug_session();
    }
}
void WindowsDebugger::handle_excetion(bool handed) {
    if (handed) {
        g_continue_status = DBG_CONTINUE;
    } else {
        g_continue_status = DBG_EXCEPTION_NOT_HANDLED;
    }
}
} // namespace debugger