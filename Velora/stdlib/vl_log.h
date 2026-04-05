#ifndef VELORA_STD_LOG_H
#define VELORA_STD_LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

typedef enum {
    VL_LOG_DEBUG = 0,
    VL_LOG_INFO  = 1,
    VL_LOG_WARN  = 2,
    VL_LOG_ERROR = 3,
    VL_LOG_FATAL = 4
} VlLogLevel;

static VlLogLevel vl_log_min_level = VL_LOG_DEBUG;
static FILE* vl_log_file = NULL;

static inline void vl_log_set_level(VlLogLevel level) {
    vl_log_min_level = level;
}

static inline void vl_log_set_file(const char* path) {
    if (vl_log_file && vl_log_file != stdout && vl_log_file != stderr) {
        fclose(vl_log_file);
    }
    vl_log_file = fopen(path, "a");
}

static inline const char* vl_log_level_str(VlLogLevel level) {
    switch (level) {
        case VL_LOG_DEBUG: return "DEBUG";
        case VL_LOG_INFO:  return "INFO ";
        case VL_LOG_WARN:  return "WARN ";
        case VL_LOG_ERROR: return "ERROR";
        case VL_LOG_FATAL: return "FATAL";
        default:           return "?????";
    }
}

static inline const char* vl_log_level_color(VlLogLevel level) {
    switch (level) {
        case VL_LOG_DEBUG: return "\033[36m";
        case VL_LOG_INFO:  return "\033[32m";
        case VL_LOG_WARN:  return "\033[33m";
        case VL_LOG_ERROR: return "\033[31m";
        case VL_LOG_FATAL: return "\033[35m";
        default:           return "\033[0m";
    }
}

static inline void vl_log(VlLogLevel level, const char* fmt, ...) {
    if (level < vl_log_min_level) return;

    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_buf[32];
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", tm_info);

    FILE* out = (level >= VL_LOG_ERROR) ? stderr : stdout;

    fprintf(out, "%s[%s] [%s]\033[0m ",
            vl_log_level_color(level),
            time_buf,
            vl_log_level_str(level));

    va_list args;
    va_start(args, fmt);
    vfprintf(out, fmt, args);
    va_end(args);

    fprintf(out, "\n");

    if (vl_log_file) {
        fprintf(vl_log_file, "[%s] [%s] ", time_buf, vl_log_level_str(level));
        va_start(args, fmt);
        vfprintf(vl_log_file, fmt, args);
        va_end(args);
        fprintf(vl_log_file, "\n");
        fflush(vl_log_file);
    }
}

static inline void vl_log_debug(const char* fmt, ...) {
    if (VL_LOG_DEBUG < vl_log_min_level) return;
    va_list args; va_start(args, fmt);
    printf("\033[36m[DEBUG]\033[0m "); vprintf(fmt, args); printf("\n");
    va_end(args);
}

static inline void vl_log_info(const char* fmt, ...) {
    if (VL_LOG_INFO < vl_log_min_level) return;
    va_list args; va_start(args, fmt);
    printf("\033[32m[INFO]\033[0m  "); vprintf(fmt, args); printf("\n");
    va_end(args);
}

static inline void vl_log_warn(const char* fmt, ...) {
    if (VL_LOG_WARN < vl_log_min_level) return;
    va_list args; va_start(args, fmt);
    fprintf(stderr, "\033[33m[WARN]\033[0m  "); vfprintf(stderr, fmt, args); fprintf(stderr, "\n");
    va_end(args);
}

static inline void vl_log_error(const char* fmt, ...) {
    if (VL_LOG_ERROR < vl_log_min_level) return;
    va_list args; va_start(args, fmt);
    fprintf(stderr, "\033[31m[ERROR]\033[0m "); vfprintf(stderr, fmt, args); fprintf(stderr, "\n");
    va_end(args);
}

#endif
