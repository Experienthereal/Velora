#ifndef VELORA_STD_MATH_H
#define VELORA_STD_MATH_H

#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define VL_PI    3.14159265358979323846
#define VL_TAU   6.28318530717958647692
#define VL_E     2.71828182845904523536
#define VL_DEG_TO_RAD 0.01745329251994329577
#define VL_RAD_TO_DEG 57.2957795130823208768

typedef struct { double x; double y; } VlVector2;
typedef struct { double x; double y; double z; } VlVector3;
typedef struct { double x; double y; double z; double w; } VlVector4;
typedef struct { double m[9]; } VlMatrix3;
typedef struct { double m[16]; } VlMatrix4;
typedef struct { double x; double y; double z; double w; } VlQuaternion;
typedef struct { uint8_t r; uint8_t g; uint8_t b; uint8_t a; } VlColor;

// ─── Vector2 ───

static inline VlVector2 vl_vec2(double x, double y) {
    VlVector2 v; v.x = x; v.y = y; return v;
}

static inline VlVector2 vl_vec2_add(VlVector2 a, VlVector2 b) {
    return vl_vec2(a.x + b.x, a.y + b.y);
}

static inline VlVector2 vl_vec2_sub(VlVector2 a, VlVector2 b) {
    return vl_vec2(a.x - b.x, a.y - b.y);
}

static inline VlVector2 vl_vec2_scale(VlVector2 v, double s) {
    return vl_vec2(v.x * s, v.y * s);
}

static inline double vl_vec2_dot(VlVector2 a, VlVector2 b) {
    return a.x * b.x + a.y * b.y;
}

static inline double vl_vec2_length(VlVector2 v) {
    return sqrt(v.x * v.x + v.y * v.y);
}

static inline double vl_vec2_distance(VlVector2 a, VlVector2 b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}

static inline VlVector2 vl_vec2_normalize(VlVector2 v) {
    double len = vl_vec2_length(v);
    if (len < 1e-12) return vl_vec2(0, 0);
    return vl_vec2(v.x / len, v.y / len);
}

static inline VlVector2 vl_vec2_lerp(VlVector2 a, VlVector2 b, double t) {
    return vl_vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
}

static inline VlVector2 vl_vec2_rotate(VlVector2 v, double angle_rad) {
    double c = cos(angle_rad);
    double s = sin(angle_rad);
    return vl_vec2(v.x * c - v.y * s, v.x * s + v.y * c);
}

// ─── Vector3 ───

static inline VlVector3 vl_vec3(double x, double y, double z) {
    VlVector3 v; v.x = x; v.y = y; v.z = z; return v;
}

static inline VlVector3 vl_vec3_add(VlVector3 a, VlVector3 b) {
    return vl_vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline VlVector3 vl_vec3_sub(VlVector3 a, VlVector3 b) {
    return vl_vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline VlVector3 vl_vec3_scale(VlVector3 v, double s) {
    return vl_vec3(v.x * s, v.y * s, v.z * s);
}

static inline double vl_vec3_dot(VlVector3 a, VlVector3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline VlVector3 vl_vec3_cross(VlVector3 a, VlVector3 b) {
    return vl_vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

static inline double vl_vec3_length(VlVector3 v) {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

static inline VlVector3 vl_vec3_normalize(VlVector3 v) {
    double len = vl_vec3_length(v);
    if (len < 1e-12) return vl_vec3(0, 0, 0);
    return vl_vec3(v.x / len, v.y / len, v.z / len);
}

static inline VlVector3 vl_vec3_lerp(VlVector3 a, VlVector3 b, double t) {
    return vl_vec3(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t);
}

// ─── Color ───

static inline VlColor vl_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    VlColor c; c.r = r; c.g = g; c.b = b; c.a = a; return c;
}

static inline VlColor vl_color_from_hex(uint32_t hex) {
    VlColor c;
    c.r = (hex >> 16) & 0xFF;
    c.g = (hex >> 8) & 0xFF;
    c.b = hex & 0xFF;
    c.a = 255;
    return c;
}

static inline VlColor vl_color_lerp(VlColor a, VlColor b, double t) {
    return vl_color(
        (uint8_t)(a.r + (b.r - a.r) * t),
        (uint8_t)(a.g + (b.g - a.g) * t),
        (uint8_t)(a.b + (b.b - a.b) * t),
        (uint8_t)(a.a + (b.a - a.a) * t)
    );
}

// ─── Random ───

static int vl_random_seeded = 0;

static inline void vl_random_seed(uint32_t seed) {
    srand(seed);
    vl_random_seeded = 1;
}

static inline void vl_random_init() {
    if (!vl_random_seeded) {
        srand((unsigned)time(NULL));
        vl_random_seeded = 1;
    }
}

static inline double vl_random() {
    vl_random_init();
    return (double)rand() / (double)RAND_MAX;
}

static inline int vl_randint(int min_val, int max_val) {
    vl_random_init();
    return min_val + rand() % (max_val - min_val + 1);
}

static inline double vl_random_range(double min_val, double max_val) {
    return min_val + vl_random() * (max_val - min_val);
}

// ─── Utility ───

static inline double vl_clamp(double value, double min_val, double max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

static inline double vl_lerp(double a, double b, double t) {
    return a + (b - a) * t;
}

static inline double vl_map(double value, double in_min, double in_max, double out_min, double out_max) {
    return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min);
}

static inline int vl_sign(double v) {
    if (v > 0) return 1;
    if (v < 0) return -1;
    return 0;
}

// ─── Geometry ───

static inline int vl_point_in_rect(double px, double py, double rx, double ry, double rw, double rh) {
    return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
}

static inline int vl_point_in_circle(double px, double py, double cx, double cy, double radius) {
    double dx = px - cx;
    double dy = py - cy;
    return (dx * dx + dy * dy) <= (radius * radius);
}

static inline int vl_circles_collide(double x1, double y1, double r1, double x2, double y2, double r2) {
    double dx = x2 - x1;
    double dy = y2 - y1;
    double dist_sq = dx * dx + dy * dy;
    double radii = r1 + r2;
    return dist_sq <= radii * radii;
}

static inline int vl_rects_collide(double x1, double y1, double w1, double h1,
                                    double x2, double y2, double w2, double h2) {
    return x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2;
}

// ─── Perlin Noise (Simple) ───

static inline double vl_noise_fade(double t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

static inline double vl_noise_grad(int hash, double x) {
    return (hash & 1) ? x : -x;
}

static inline double vl_noise_1d(double x) {
    int xi = (int)floor(x) & 255;
    double xf = x - floor(x);
    double u = vl_noise_fade(xf);
    int a = (xi * 2654435761u) & 255;
    int b = ((xi + 1) * 2654435761u) & 255;
    return vl_lerp(vl_noise_grad(a, xf), vl_noise_grad(b, xf - 1), u);
}

#endif
