// Based on: https://github.com/rxi/log.c/

#pragma once

#include "common.h"

#include <stdio.h>
#include <stdarg.h>

typedef void (*log_LockProc)(b32 should_lock);

typedef enum {
    LogLevelTrace = 0,
    LogLevelDebug = 1,
    LogLevelInfo  = 2,
    LogLevelWarn  = 3,
    LogLevelError = 4,
    LogLevelFatal = 5,
} LogLevelE;

#define log_trace(...) log_log(LogLevelTrace, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LogLevelDebug, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) log_log(LogLevelInfo, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) log_log(LogLevelWarn, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LogLevelError, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LogLevelFatal, __FILE__, __LINE__, __VA_ARGS__)

const char *log_get_level();

void log_set_level(LogLevelE lvl);
void log_set_quiet(b32 quiet);
void  log_set_lock(log_LockProc proc);
void    log_set_fp(FILE *log_file);

void log_log(LogLevelE lvl, const char *src_file, u64 src_line, const char *fmt, ...);
