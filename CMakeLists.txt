cmake_minimum_required(VERSION 3.22.1)
# 设置编译器为gcc/g++
# SET(CMAKE_C_COMPILER /usr/bin/gcc)
# SET(CMAKE_CXX_COMPILER /usr/bin/g++)

set(CMAKE_CXX_STANDARD 20)
# 设置必须跟上面的一样，即c++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置生成 compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(c++_debugger)

include_directories(${CMAKE_SOURCE_DIR}/src/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/linenoise)
# 设置生成 compile_commands

# 显示编译时间
set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CMAKE_COMMAND} -E time")

# 设置成debug模式
# set(CMAKE_BUILD_TYPE "Debug")
# 设置调试的指令
if (MSVC)
    # Unicode编码
    add_executable(debugger main.cpp)
    add_compile_options(/W4)

    add_library(debugger_libs STATIC 
    ${CMAKE_SOURCE_DIR}/src/windows/debugger/windows_debugger.cc
    # ${CMAKE_SOURCE_DIR}/src/debugger/break_point.cc
    # ${CMAKE_SOURCE_DIR}/src/debugger/ptrace_expr_context.cc
    )
    
    target_link_libraries(debugger PRIVATE debugger_libs)
    
else()
    add_compile_options(-Wall -Wextra -Wshadow -Wpedantic) # fsanitize=undefined -fsanitize=address
    # add_compile_options(-Wall -Wextra)


    # 加入linenoise 和 libelfin 的头文件，这样可能出现偏差，但是我觉得还行（

    add_library(linenoise_libs STATIC
    ${CMAKE_SOURCE_DIR}/third_party/linenoise/linenoise.c
    )

    add_executable(debugger main.cpp)

    add_library(debugger_libs STATIC 
        ${CMAKE_SOURCE_DIR}/src/debugger/debugger.cc
        ${CMAKE_SOURCE_DIR}/src/debugger/break_point.cc
        ${CMAKE_SOURCE_DIR}/src/debugger/ptrace_expr_context.cc
    )


    add_subdirectory(third_party/fmt)
    # fmt头文件
    include_directories(${CMAKE_SOURCE_DIR}/third_party/fmt/include)


    # 链接 linenoise
    target_link_libraries(debugger PRIVATE debugger_libs)
    target_link_libraries(debugger PRIVATE linenoise_libs)
    target_link_libraries(debugger PRIVATE /usr/local/lib/libelf++.so)
    target_link_libraries(debugger PRIVATE /usr/local/lib/libdwarf++.so)
    target_link_libraries(debugger PRIVATE fmt)

endif()

