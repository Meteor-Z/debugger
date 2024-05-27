#if defined(__linux__)

#include "sys/personality.h"
#include <sys/ptrace.h>
#include <unistd.h>
#include <sched.h>
#include "debugger/debugger.h"

#endif

#if defined(_WIN32)

#include <Windows.h>
#include <winuser.h>
#include <debugapi.h>
#include <minwinbase.h>
#include <processthreadsapi.h>
#include <errhandlingapi.h>
#include <handleapi.h>
#include <winbase.h>
#include <winnt.h>
#include <memoryapi.h>
#include <minwindef.h>
#include <oleidl.h>
#include "src/windows/include/debugger/windows_debugger.h"
#include <iostream>

#endif

#if defined(_WIN32)

int main(int argc, char* argv[]) {
    STARTUPINFO si = {0};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {0};

    if (CreateProcess(TEXT("C:\\windows\\notepade.exe"),
            NULL,
            NULL,
            NULL,
            FALSE,
            DEBUG_ONLY_THIS_PROCESS |
                CREATE_NEW_CONSOLE, // 只调试他这个进程，不调试他的子进程，并且子进程新开一个窗口，否则输出会乱
            NULL,
            NULL,
            &si,
            &pi) == FALSE) {
        std::cout << "CreateProcess failed " << GetLastError() << std::endl;
        return -1;
    }

    bool wait_event = true;
    DEBUG_EVENT debug_event{}; // 调试事件

    debugger::WindowsDebugger debugger{pi.hProcess};

    // 循环
    while (wait_event == true && WaitForDebugEvent(&debug_event, INFINITE)) {

        switch (debug_event.dwDebugEventCode) {
        case CREATE_PROCESS_DEBUG_EVENT:
            debugger.OnProcessCreated(&debug_event.u.CreateProcessInfo);
            break;
        case CREATE_THREAD_DEBUG_EVENT:
            debugger.OnThreadCreated(&debug_event.u.CreateThread);
            break;
        case EXCEPTION_DEBUG_EVENT:
            debugger.OnException(&debug_event.u.Exception);
            break;
        case EXIT_PROCESS_DEBUG_EVENT:
            debugger.OnProcessExited(&debug_event.u.ExitProcess);
            wait_event = false;
            break;
        case LOAD_DLL_DEBUG_EVENT:
            debugger.OnDllLoaded(&debug_event.u.LoadDll);
            break;
        case UNLOAD_DLL_DEBUG_EVENT:
            debugger.OnDllUnloaded(&debug_event.u.UnloadDll);
            break;
        case OUTPUT_DEBUG_STRING_EVENT:
            debugger.on_output_debug_string(&debug_event.u.DebugString);
            break;
        case RIP_EVENT:
            debugger.OnRipEvent(&debug_event.u.RipInfo);
            break;
        default:
            std::cout << "UnKnown debug event." << std::endl;
            break;
        }

        if (wait_event == true) {
            ContinueDebugEvent(
                debug_event.dwProcessId, debug_event.dwThreadId, DBG_CONTINUE);

        } else {
            break;
        }
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    std::cout << "yes" << std::endl;
}
#endif

#if defined(__linux__)

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "错误" << std::endl;
        return -1;
    }

    std::string program_name{argv[1]};
    pid_t pid = fork();

    // std::cout << argv[1] << std::endl;

    // childen
    if (pid == 0) {
        personality(ADDR_NO_RANDOMIZE);
        // extern long int ptrace (enum __ptrace_request __request, ...);
        // 设置跟踪状态并且执行
        ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
        execl(program_name.c_str(), program_name.c_str(), nullptr);
    }

    if (pid > 0) {
        std::cout << "开始Debug" << std::endl;
        my_gdb::Debugger gdb{program_name, pid};
        gdb.run();
    }
}

#endif