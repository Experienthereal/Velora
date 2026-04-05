#ifndef VELORAGAME_H
#define VELORAGAME_H

#include "../stdlib/vl_math.h"
#include "../stdlib/vl_time.h"
#include "../stdlib/vl_io.h"
#include "../stdlib/vl_log.h"
#include <stdint.h>

// ─── Graphics Backend Selection ───

typedef enum {
    VG_BACKEND_AUTO = 0,
    VG_BACKEND_OPENGL,
    VG_BACKEND_VULKAN,
    VG_BACKEND_DIRECTX
} VgBackend;

// ─── Forward Declarations ───

typedef struct VgWindow VgWindow;
typedef struct VgRenderer VgRenderer;
typedef struct VgSprite VgSprite;
typedef struct VgTexture VgTexture;
typedef struct VgFont VgFont;
typedef struct VgSound VgSound;

// ─── Event System ───

typedef enum {
    VG_EVENT_NONE = 0,
    VG_EVENT_QUIT,
    VG_EVENT_KEY_DOWN,
    VG_EVENT_KEY_UP,
    VG_EVENT_MOUSE_MOVE,
    VG_EVENT_MOUSE_DOWN,
    VG_EVENT_MOUSE_UP,
    VG_EVENT_MOUSE_WHEEL,
    VG_EVENT_RESIZE,
    VG_EVENT_FOCUS,
    VG_EVENT_BLUR
} VgEventType;

typedef struct {
    VgEventType type;
    int key_code;
    int mouse_button;
    int mouse_x;
    int mouse_y;
    int wheel_delta;
    int width;
    int height;
} VgEvent;

// ─── Key Codes ───

typedef enum {
    VG_KEY_UNKNOWN = 0,
    VG_KEY_A = 'A', VG_KEY_B = 'B', VG_KEY_C = 'C', VG_KEY_D = 'D',
    VG_KEY_E = 'E', VG_KEY_F = 'F', VG_KEY_G = 'G', VG_KEY_H = 'H',
    VG_KEY_I = 'I', VG_KEY_J = 'J', VG_KEY_K = 'K', VG_KEY_L = 'L',
    VG_KEY_M = 'M', VG_KEY_N = 'N', VG_KEY_O = 'O', VG_KEY_P = 'P',
    VG_KEY_Q = 'Q', VG_KEY_R = 'R', VG_KEY_S = 'S', VG_KEY_T = 'T',
    VG_KEY_U = 'U', VG_KEY_V = 'V', VG_KEY_W = 'W', VG_KEY_X = 'X',
    VG_KEY_Y = 'Y', VG_KEY_Z = 'Z',
    VG_KEY_0 = '0', VG_KEY_1 = '1', VG_KEY_2 = '2', VG_KEY_3 = '3',
    VG_KEY_4 = '4', VG_KEY_5 = '5', VG_KEY_6 = '6', VG_KEY_7 = '7',
    VG_KEY_8 = '8', VG_KEY_9 = '9',
    VG_KEY_SPACE = 32,
    VG_KEY_ENTER = 13,
    VG_KEY_ESCAPE = 27,
    VG_KEY_TAB = 9,
    VG_KEY_BACKSPACE = 8,
    VG_KEY_UP = 256,
    VG_KEY_DOWN = 257,
    VG_KEY_LEFT = 258,
    VG_KEY_RIGHT = 259,
    VG_KEY_SHIFT = 260,
    VG_KEY_CTRL = 261,
    VG_KEY_ALT = 262,
    VG_KEY_F1 = 270, VG_KEY_F2, VG_KEY_F3, VG_KEY_F4,
    VG_KEY_F5, VG_KEY_F6, VG_KEY_F7, VG_KEY_F8,
    VG_KEY_F9, VG_KEY_F10, VG_KEY_F11, VG_KEY_F12
} VgKeyCode;

// ─── Mouse ───

typedef enum {
    VG_MOUSE_LEFT = 0,
    VG_MOUSE_RIGHT = 1,
    VG_MOUSE_MIDDLE = 2
} VgMouseButton;

// ─── Rect ───

typedef struct {
    double x, y, w, h;
} VgRect;

static inline VgRect vg_rect(double x, double y, double w, double h) {
    VgRect r; r.x = x; r.y = y; r.w = w; r.h = h; return r;
}

// ─── Veloragame Core API ───

int         vg_init(VgBackend backend);
void        vg_quit();
const char* vg_backend_name();

// Window
VgWindow*   vg_window_create(const char* title, int width, int height, int fullscreen);
void        vg_window_destroy(VgWindow* window);
int         vg_window_is_open(VgWindow* window);
void        vg_window_close(VgWindow* window);
void        vg_window_set_title(VgWindow* window, const char* title);
int         vg_window_width(VgWindow* window);
int         vg_window_height(VgWindow* window);
int         vg_window_poll_event(VgWindow* window, VgEvent* event);

// Renderer
VgRenderer* vg_renderer_create(VgWindow* window);
void        vg_renderer_destroy(VgRenderer* renderer);
void        vg_renderer_clear(VgRenderer* renderer, VlColor color);
void        vg_renderer_present(VgRenderer* renderer);
void        vg_renderer_draw_rect(VgRenderer* renderer, VgRect rect, VlColor color);
void        vg_renderer_draw_rect_outline(VgRenderer* renderer, VgRect rect, VlColor color, float thickness);
void        vg_renderer_draw_circle(VgRenderer* renderer, double cx, double cy, double radius, VlColor color);
void        vg_renderer_draw_line(VgRenderer* renderer, double x1, double y1, double x2, double y2, VlColor color, float thickness);
void        vg_renderer_draw_sprite(VgRenderer* renderer, VgSprite* sprite);
void        vg_renderer_draw_text(VgRenderer* renderer, const char* text, double x, double y, VlColor color);

// Texture
VgTexture*  vg_texture_load(VgRenderer* renderer, const char* path);
void        vg_texture_destroy(VgTexture* texture);
int         vg_texture_width(VgTexture* texture);
int         vg_texture_height(VgTexture* texture);

// Sprite
VgSprite*   vg_sprite_create(VgTexture* texture);
VgSprite*   vg_sprite_create_from_file(VgRenderer* renderer, const char* path);
void        vg_sprite_destroy(VgSprite* sprite);
void        vg_sprite_set_position(VgSprite* sprite, double x, double y);
void        vg_sprite_set_scale(VgSprite* sprite, double sx, double sy);
void        vg_sprite_set_rotation(VgSprite* sprite, double angle);
void        vg_sprite_set_color(VgSprite* sprite, VlColor color);
void        vg_sprite_set_visible(VgSprite* sprite, int visible);
void        vg_sprite_flip_x(VgSprite* sprite);
void        vg_sprite_flip_y(VgSprite* sprite);
VlVector2   vg_sprite_position(VgSprite* sprite);

// Input
int         vg_key_pressed(int key_code);
int         vg_key_just_pressed(int key_code);
int         vg_key_released(int key_code);
VlVector2   vg_mouse_position();
int         vg_mouse_pressed(VgMouseButton button);
int         vg_mouse_just_pressed(VgMouseButton button);
void        vg_input_update();

// Camera
void        vg_camera_set_position(VgRenderer* renderer, double x, double y);
void        vg_camera_set_zoom(VgRenderer* renderer, double zoom);

#endif
