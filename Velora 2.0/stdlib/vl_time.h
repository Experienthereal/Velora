#ifndef VELORA_STD_TIME_H
#define VELORA_STD_TIME_H

#include <stdint.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <time.h>
    #include <sys/time.h>
#endif

// ─── High-Resolution Timer ───

typedef struct {
    double start_time;
    double last_frame;
    double delta;
    double elapsed;
    int64_t frame_count;
    double fps;
    double target_fps;
    double accumulator;
} VlTimer;

static inline double vl_time_now() {
    #ifdef _WIN32
        LARGE_INTEGER freq, counter;
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&counter);
        return (double)counter.QuadPart / (double)freq.QuadPart;
    #else
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
    #endif
}

static inline VlTimer vl_timer_create() {
    VlTimer t;
    t.start_time = vl_time_now();
    t.last_frame = t.start_time;
    t.delta = 0.0;
    t.elapsed = 0.0;
    t.frame_count = 0;
    t.fps = 0.0;
    t.target_fps = 60.0;
    t.accumulator = 0.0;
    return t;
}

static inline void vl_timer_tick(VlTimer* t) {
    double now = vl_time_now();
    t->delta = now - t->last_frame;
    t->last_frame = now;
    t->elapsed = now - t->start_time;
    t->frame_count++;

    t->accumulator += t->delta;
    if (t->accumulator >= 1.0) {
        t->fps = (double)t->frame_count / t->accumulator;
        t->frame_count = 0;
        t->accumulator = 0.0;
    }
}

static inline double vl_timer_delta(VlTimer* t) {
    return t->delta;
}

static inline double vl_timer_fps(VlTimer* t) {
    return t->fps;
}

static inline double vl_timer_elapsed(VlTimer* t) {
    return t->elapsed;
}

// ─── Sleep ───

static inline void vl_sleep_ms(int milliseconds) {
    #ifdef _WIN32
        Sleep(milliseconds);
    #else
        struct timespec ts;
        ts.tv_sec = milliseconds / 1000;
        ts.tv_nsec = (milliseconds % 1000) * 1000000;
        nanosleep(&ts, NULL);
    #endif
}

static inline void vl_sleep(double seconds) {
    vl_sleep_ms((int)(seconds * 1000.0));
}

// ─── Stopwatch ───

typedef struct {
    double start;
    double accumulated;
    int running;
} VlStopwatch;

static inline VlStopwatch vl_stopwatch_create() {
    VlStopwatch sw;
    sw.start = 0;
    sw.accumulated = 0;
    sw.running = 0;
    return sw;
}

static inline void vl_stopwatch_start(VlStopwatch* sw) {
    if (!sw->running) {
        sw->start = vl_time_now();
        sw->running = 1;
    }
}

static inline void vl_stopwatch_stop(VlStopwatch* sw) {
    if (sw->running) {
        sw->accumulated += vl_time_now() - sw->start;
        sw->running = 0;
    }
}

static inline void vl_stopwatch_reset(VlStopwatch* sw) {
    sw->accumulated = 0;
    sw->running = 0;
}

static inline double vl_stopwatch_elapsed(VlStopwatch* sw) {
    if (sw->running) {
        return sw->accumulated + (vl_time_now() - sw->start);
    }
    return sw->accumulated;
}

#endif
