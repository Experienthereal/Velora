#ifndef VELORA_STD_FS_H
#define VELORA_STD_FS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
    #include <io.h>
    #include <direct.h>
    #define VL_PATH_SEP '\\'
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #include <dirent.h>
    #define VL_PATH_SEP '/'
#endif

// ─── File Reading ───

static inline char* vl_fs_read_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buffer = (char*)malloc(size + 1);
    if (!buffer) { fclose(f); return NULL; }

    fread(buffer, 1, size, f);
    buffer[size] = '\0';
    fclose(f);
    return buffer;
}

static inline uint8_t* vl_fs_read_bytes(const char* path, long* out_size) {
    FILE* f = fopen(path, "rb");
    if (!f) { *out_size = 0; return NULL; }

    fseek(f, 0, SEEK_END);
    *out_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t* buffer = (uint8_t*)malloc(*out_size);
    fread(buffer, 1, *out_size, f);
    fclose(f);
    return buffer;
}

// ─── File Writing ───

static inline int vl_fs_write_file(const char* path, const char* content) {
    FILE* f = fopen(path, "w");
    if (!f) return 0;
    fputs(content, f);
    fclose(f);
    return 1;
}

static inline int vl_fs_write_bytes(const char* path, const uint8_t* data, long size) {
    FILE* f = fopen(path, "wb");
    if (!f) return 0;
    fwrite(data, 1, size, f);
    fclose(f);
    return 1;
}

static inline int vl_fs_append_file(const char* path, const char* content) {
    FILE* f = fopen(path, "a");
    if (!f) return 0;
    fputs(content, f);
    fclose(f);
    return 1;
}

// ─── File Operations ───

static inline int vl_fs_exists(const char* path) {
    FILE* f = fopen(path, "r");
    if (f) { fclose(f); return 1; }
    return 0;
}

static inline int vl_fs_delete(const char* path) {
    return remove(path) == 0;
}

static inline int vl_fs_rename(const char* old_path, const char* new_path) {
    return rename(old_path, new_path) == 0;
}

// ─── Path Utilities ───

static inline const char* vl_fs_extension(const char* path) {
    const char* dot = strrchr(path, '.');
    if (!dot || dot == path) return "";
    return dot;
}

static inline char* vl_fs_basename(const char* path) {
    const char* last_sep = strrchr(path, VL_PATH_SEP);
    #ifdef _WIN32
        const char* last_fwd = strrchr(path, '/');
        if (last_fwd && (!last_sep || last_fwd > last_sep)) last_sep = last_fwd;
    #endif
    if (!last_sep) return strdup(path);
    return strdup(last_sep + 1);
}

static inline char* vl_fs_dirname(const char* path) {
    const char* last_sep = strrchr(path, VL_PATH_SEP);
    #ifdef _WIN32
        const char* last_fwd = strrchr(path, '/');
        if (last_fwd && (!last_sep || last_fwd > last_sep)) last_sep = last_fwd;
    #endif
    if (!last_sep) return strdup(".");
    size_t len = last_sep - path;
    char* result = (char*)malloc(len + 1);
    memcpy(result, path, len);
    result[len] = '\0';
    return result;
}

static inline char* vl_fs_join(const char* a, const char* b) {
    size_t la = strlen(a);
    size_t lb = strlen(b);
    char* result = (char*)malloc(la + lb + 2);
    memcpy(result, a, la);
    result[la] = VL_PATH_SEP;
    memcpy(result + la + 1, b, lb + 1);
    return result;
}

// ─── Streaming File Handle ───

typedef struct {
    FILE* handle;
    char* path;
    int is_open;
} VlFile;

static inline VlFile vl_file_open(const char* path, const char* mode) {
    VlFile f;
    f.handle = fopen(path, mode);
    f.path = strdup(path);
    f.is_open = f.handle != NULL;
    return f;
}

static inline char* vl_file_read_line(VlFile* f) {
    static char buffer[4096];
    if (!f->is_open || !f->handle) return NULL;
    if (fgets(buffer, sizeof(buffer), f->handle)) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = '\0';
        return buffer;
    }
    return NULL;
}

static inline char* vl_file_read_all(VlFile* f) {
    if (!f->is_open || !f->handle) return NULL;
    fseek(f->handle, 0, SEEK_END);
    long size = ftell(f->handle);
    fseek(f->handle, 0, SEEK_SET);
    char* buffer = (char*)malloc(size + 1);
    fread(buffer, 1, size, f->handle);
    buffer[size] = '\0';
    return buffer;
}

static inline int vl_file_write(VlFile* f, const char* content) {
    if (!f->is_open || !f->handle) return 0;
    fputs(content, f->handle);
    return 1;
}

static inline void vl_file_close(VlFile* f) {
    if (f->handle) { fclose(f->handle); f->handle = NULL; }
    if (f->path) { free(f->path); f->path = NULL; }
    f->is_open = 0;
}

#endif
