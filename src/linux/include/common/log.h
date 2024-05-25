/**
 * @file log.h
 * @author lzc (liuzechen.coder@qq.com)
 * @brief 十分简单的 log 调试库
 * @version 0.1
 * @date 2024-01-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#ifndef MY_GDB_COMMON_LOG_H
#define MY_GDB_COMMON_LOG_H

#include "fmt/core.h"

/**
 * @brief debug的调试库
 * 
 */
#define DEBUG_LOG(message)                                                               \
    fmt::print("[file: {}:{}] [info = {}]\n", __FILE__, __LINE__, message);


/**
 * @brief ERROR的调试库
 * 
 */
#define ERROR_LOG(message)                                                               \
    fmt::print("[file: {}:{}] [info = {}]\n", __FILE__, __LINE__, message);

#endif