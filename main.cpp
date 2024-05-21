
#if defined(__linux__)
#include "sys/personality.h"
#include <sys/ptrace.h>
#include <unistd.h>
#include <sched.h>
#include "debugger/debugger.h"
#endif

#if defined(_WIN32)

#include <Windows.h>
#include <processthreadsapi.h>
#include <errhandlingapi.h>
#include <handleapi.h>
#include <iostream>
#include <winbase.h>
#include <winnt.h>

#endif

#if defined(_WIN32)

int main(int argc, char* argv[]) {
    STARTUPINFO si = { 0 };
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi = { 0 };

    if (CreateProcess(TEXT("C:\\windows\\notepade.exe"), NULL, NULL, NULL, FALSE,
                      DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi) == FALSE) {
        std::wcout << TEXT("CreateProcess failed ") << GetLastError() << std::endl;
        return -1;
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

    std::string program_name { argv[1] };
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
        my_gdb::Debugger gdb { program_name, pid };
        gdb.run();
    }
}

#endif