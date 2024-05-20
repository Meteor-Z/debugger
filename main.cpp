#if defined (__linux__)
    #include "sys/personality.h"
    #include <sys/ptrace.h>
    #include <unistd.h>
    #include <sched.h>
    #include "debugger/debugger.h"
#endif

#include <iostream>

#if defined(_WIN32)

int main(int argc, char* argv[]) {
    std::cout << "Hello Wolrd" << std::endl;
}

#endif

#if defined (__linux__)


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