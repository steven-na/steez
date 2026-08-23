#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

static struct {
    log_LockProc lock_proc;
    LogLevelE lvl;
    b32 quiet;
    b32 inited;
    FILE *log_default;
    FILE *log_file;
} L;

static const char *level_strings[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifndef LOG_NO_COLOR
static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif

const char *log_get_level() {
    return level_strings[L.lvl];
}

void log_set_level(LogLevelE lvl) {
    L.lvl = lvl;
}

void log_init(FILE *out) {
    L.inited = true;
    L.log_default = out;
}

void log_set_quiet(b32 quiet) {
    L.quiet = quiet;
}

void log_set_lock(log_LockProc proc) {
    L.lock_proc = proc;
}

void log_set_fp(FILE *log_file) {
    L.log_file = log_file;
}

static inline void   lock() { if (L.lock_proc) L.lock_proc( true); }
static inline void unlock() { if (L.lock_proc) L.lock_proc(false); }

static inline void write_to_file(LogLevelE lvl, const char *src_file, u64 src_line, const char *fmt, struct tm* time, va_list va) {
    if (!L.log_file) { return; }
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time);

    fprintf(L.log_file, "%s [%-5s] %s:%lu ", time_str, level_strings[lvl], src_file, src_line);
    vfprintf(L.log_file, fmt, va);
    fprintf(L.log_file, "\n");
}

void log_log(LogLevelE lvl, const char *src_file, u64 src_line, const char *fmt, ...) {
    if (!L.inited) return;

    lock();

    time_t t = time(NULL);
    struct tm tm_info;
    struct tm* time_ptr = localtime_r(&t, &tm_info);

    bool should_log_to_console = !(L.quiet || lvl < L.lvl);

    va_list va;
    va_start(va, fmt);

    if (should_log_to_console) {
        char time_str[16];
        strftime(time_str, sizeof(time_str), "%H:%M:%S", time_ptr);

#ifndef LOG_NO_COLOR
        fprintf(L.log_default, "%s %s[%-5s] \x1b[0m\x1b[90m%s:%lu:\x1b[0m ",
            time_str, level_colors[lvl], level_strings[lvl], src_file, src_line);
#else
        fprintf(L.log_default, "%s %-5s %s:%lu: ",
            time_str, level_strings[L.lvl], src_file, src_line);
#endif

        vfprintf(L.log_default, fmt, va);
        fprintf(L.log_default, "\n");
    }

    va_list fva;
    va_copy(fva, va);
    va_end(va);

    write_to_file(lvl, src_file, src_line, fmt, time_ptr, fva);
    va_end(fva);

    unlock();
}
