#ifndef VELORA_STD_IO_H
#define VELORA_STD_IO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// ─── Print ───

static inline void vl_print_str(const char* s) {
    printf("%s\n", s);
}

static inline void vl_print_int(int64_t n) {
    printf("%lld\n", (long long)n);
}

static inline void vl_print_double(double d) {
    printf("%g\n", d);
}

static inline void vl_print_bool(int b) {
    printf("%s\n", b ? "true" : "false");
}

static inline void vl_print_char(char c) {
    printf("%c\n", c);
}

static inline void vl_print_no_newline(const char* s) {
    printf("%s", s);
}

// ─── Input ───

static inline char* vl_input(const char* prompt) {
    static char buffer[4096];
    if (prompt && prompt[0]) {
        printf("%s", prompt);
    }
    if (fgets(buffer, sizeof(buffer), stdin)) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        return buffer;
    }
    return "";
}

// ─── String Utilities ───

static inline int vl_str_length(const char* s) {
    return (int)strlen(s);
}

static inline int vl_str_equals(const char* a, const char* b) {
    return strcmp(a, b) == 0;
}

static inline int vl_str_contains(const char* haystack, const char* needle) {
    return strstr(haystack, needle) != NULL;
}

static inline int vl_str_starts_with(const char* str, const char* prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

static inline int vl_str_ends_with(const char* str, const char* suffix) {
    size_t str_len = strlen(str);
    size_t suf_len = strlen(suffix);
    if (suf_len > str_len) return 0;
    return strcmp(str + str_len - suf_len, suffix) == 0;
}

static inline char* vl_str_concat(const char* a, const char* b) {
    size_t la = strlen(a);
    size_t lb = strlen(b);
    char* result = (char*)malloc(la + lb + 1);
    memcpy(result, a, la);
    memcpy(result + la, b, lb + 1);
    return result;
}

static inline char* vl_str_slice(const char* s, int start, int end) {
    int len = (int)strlen(s);
    if (start < 0) start = len + start;
    if (end < 0) end = len + end;
    if (start < 0) start = 0;
    if (end > len) end = len;
    if (start >= end) {
        char* empty = (char*)malloc(1);
        empty[0] = '\0';
        return empty;
    }
    int slice_len = end - start;
    char* result = (char*)malloc(slice_len + 1);
    memcpy(result, s + start, slice_len);
    result[slice_len] = '\0';
    return result;
}

static inline char* vl_int_to_str(int64_t n) {
    static char buffer[64];
    snprintf(buffer, sizeof(buffer), "%lld", (long long)n);
    return buffer;
}

static inline char* vl_double_to_str(double d) {
    static char buffer[64];
    snprintf(buffer, sizeof(buffer), "%g", d);
    return buffer;
}

static inline int64_t vl_str_to_int(const char* s) {
    return atoll(s);
}

static inline double vl_str_to_double(const char* s) {
    return atof(s);
}

#endif
