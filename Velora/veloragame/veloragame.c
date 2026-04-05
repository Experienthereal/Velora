#include "veloragame.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

// ─── Internal State ───

static VgBackend active_backend = VG_BACKEND_AUTO;
static int vg_initialized = 0;

#define VG_MAX_KEYS 512
static uint8_t key_state[VG_MAX_KEYS];
static uint8_t key_prev[VG_MAX_KEYS];
static uint8_t mouse_state[8];
static uint8_t mouse_prev[8];
static int mouse_x = 0;
static int mouse_y = 0;

// ─── Window Structure ───

struct VgWindow {
    #ifdef _WIN32
        HWND hwnd;
        HDC hdc;
        int should_close;
    #else
        void* native_handle;
        int should_close;
    #endif
    int width;
    int height;
    char title[256];
    int fullscreen;
};

// ─── Renderer Structure ───

struct VgRenderer {
    VgWindow* window;
    VgBackend backend;
    double camera_x;
    double camera_y;
    double camera_zoom;

    #ifdef _WIN32
        HDC mem_dc;
        HBITMAP bitmap;
        uint32_t* pixels;
    #endif
};

// ─── Sprite Structure ───

struct VgSprite {
    VgTexture* texture;
    VlVector2 position;
    VlVector2 scale;
    double rotation;
    VlColor color;
    int visible;
    int flip_h;
    int flip_v;
};

// ─── Texture Structure ───

struct VgTexture {
    uint32_t* pixels;
    int width;
    int height;
};

// ─── Win32 Window Procedure ───

#ifdef _WIN32

static LRESULT CALLBACK vg_wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CLOSE:
        case WM_DESTROY: {
            VgWindow* win = (VgWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if (win) win->should_close = 1;
            return 0;
        }
        case WM_KEYDOWN:
            if (wp < VG_MAX_KEYS) key_state[wp] = 1;
            return 0;
        case WM_KEYUP:
            if (wp < VG_MAX_KEYS) key_state[wp] = 0;
            return 0;
        case WM_LBUTTONDOWN: mouse_state[VG_MOUSE_LEFT] = 1; return 0;
        case WM_LBUTTONUP:   mouse_state[VG_MOUSE_LEFT] = 0; return 0;
        case WM_RBUTTONDOWN: mouse_state[VG_MOUSE_RIGHT] = 1; return 0;
        case WM_RBUTTONUP:   mouse_state[VG_MOUSE_RIGHT] = 0; return 0;
        case WM_MBUTTONDOWN: mouse_state[VG_MOUSE_MIDDLE] = 1; return 0;
        case WM_MBUTTONUP:   mouse_state[VG_MOUSE_MIDDLE] = 0; return 0;
        case WM_MOUSEMOVE:
            mouse_x = LOWORD(lp);
            mouse_y = HIWORD(lp);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

#endif

// ─── Core Init / Quit ───

int vg_init(VgBackend backend) {
    if (vg_initialized) return 1;

    active_backend = backend;
    if (backend == VG_BACKEND_AUTO) {
        #ifdef _WIN32
            active_backend = VG_BACKEND_DIRECTX;
        #else
            active_backend = VG_BACKEND_OPENGL;
        #endif
    }

    memset(key_state, 0, sizeof(key_state));
    memset(key_prev, 0, sizeof(key_prev));
    memset(mouse_state, 0, sizeof(mouse_state));
    memset(mouse_prev, 0, sizeof(mouse_prev));

    vg_initialized = 1;
    vl_log_info("Veloragame initialized [backend: %s]", vg_backend_name());
    return 1;
}

void vg_quit() {
    vg_initialized = 0;
    vl_log_info("Veloragame shutdown");
}

const char* vg_backend_name() {
    switch (active_backend) {
        case VG_BACKEND_OPENGL:  return "OpenGL";
        case VG_BACKEND_VULKAN:  return "Vulkan";
        case VG_BACKEND_DIRECTX: return "DirectX";
        default:                 return "Auto";
    }
}

// ─── Window ───

VgWindow* vg_window_create(const char* title, int width, int height, int fullscreen) {
    VgWindow* win = (VgWindow*)calloc(1, sizeof(VgWindow));
    win->width = width;
    win->height = height;
    win->should_close = 0;
    win->fullscreen = fullscreen;
    strncpy(win->title, title, sizeof(win->title) - 1);

    #ifdef _WIN32
    {
        WNDCLASSA wc = {0};
        wc.lpfnWndProc = vg_wndproc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = "VeloraGameWindow";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        RegisterClassA(&wc);

        DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
        if (fullscreen) style = WS_POPUP | WS_VISIBLE;

        RECT rect = {0, 0, width, height};
        AdjustWindowRect(&rect, style, FALSE);

        win->hwnd = CreateWindowA(
            "VeloraGameWindow", title, style,
            CW_USEDEFAULT, CW_USEDEFAULT,
            rect.right - rect.left,
            rect.bottom - rect.top,
            NULL, NULL, GetModuleHandle(NULL), NULL
        );

        SetWindowLongPtr(win->hwnd, GWLP_USERDATA, (LONG_PTR)win);
        win->hdc = GetDC(win->hwnd);
        ShowWindow(win->hwnd, SW_SHOW);
        UpdateWindow(win->hwnd);
    }
    #endif

    vl_log_info("Window created: '%s' (%dx%d)", title, width, height);
    return win;
}

void vg_window_destroy(VgWindow* window) {
    if (!window) return;
    #ifdef _WIN32
        if (window->hwnd) DestroyWindow(window->hwnd);
    #endif
    free(window);
}

int vg_window_is_open(VgWindow* window) {
    if (!window) return 0;
    return !window->should_close;
}

void vg_window_close(VgWindow* window) {
    if (window) window->should_close = 1;
}

void vg_window_set_title(VgWindow* window, const char* title) {
    if (!window) return;
    strncpy(window->title, title, sizeof(window->title) - 1);
    #ifdef _WIN32
        SetWindowTextA(window->hwnd, title);
    #endif
}

int vg_window_width(VgWindow* window) { return window ? window->width : 0; }
int vg_window_height(VgWindow* window) { return window ? window->height : 0; }

int vg_window_poll_event(VgWindow* window, VgEvent* event) {
    if (!window || !event) return 0;
    event->type = VG_EVENT_NONE;

    #ifdef _WIN32
    {
        MSG msg;
        if (PeekMessage(&msg, window->hwnd, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            switch (msg.message) {
                case WM_QUIT:
                    event->type = VG_EVENT_QUIT;
                    window->should_close = 1;
                    return 1;
                case WM_KEYDOWN:
                    event->type = VG_EVENT_KEY_DOWN;
                    event->key_code = (int)msg.wParam;
                    return 1;
                case WM_KEYUP:
                    event->type = VG_EVENT_KEY_UP;
                    event->key_code = (int)msg.wParam;
                    return 1;
                case WM_MOUSEMOVE:
                    event->type = VG_EVENT_MOUSE_MOVE;
                    event->mouse_x = LOWORD(msg.lParam);
                    event->mouse_y = HIWORD(msg.lParam);
                    return 1;
            }
            return 1;
        }
    }
    #endif

    return 0;
}

// ─── Renderer ───

VgRenderer* vg_renderer_create(VgWindow* window) {
    if (!window) return NULL;
    VgRenderer* r = (VgRenderer*)calloc(1, sizeof(VgRenderer));
    r->window = window;
    r->backend = active_backend;
    r->camera_x = 0;
    r->camera_y = 0;
    r->camera_zoom = 1.0;

    #ifdef _WIN32
    {
        BITMAPINFO bmi = {0};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = window->width;
        bmi.bmiHeader.biHeight = -window->height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        r->mem_dc = CreateCompatibleDC(window->hdc);
        r->bitmap = CreateDIBSection(r->mem_dc, &bmi, DIB_RGB_COLORS, (void**)&r->pixels, NULL, 0);
        SelectObject(r->mem_dc, r->bitmap);
    }
    #endif

    vl_log_info("Renderer created [%s]", vg_backend_name());
    return r;
}

void vg_renderer_destroy(VgRenderer* renderer) {
    if (!renderer) return;
    #ifdef _WIN32
        if (renderer->bitmap) DeleteObject(renderer->bitmap);
        if (renderer->mem_dc) DeleteDC(renderer->mem_dc);
    #endif
    free(renderer);
}

void vg_renderer_clear(VgRenderer* renderer, VlColor color) {
    if (!renderer || !renderer->pixels) return;
    int total = renderer->window->width * renderer->window->height;
    uint32_t pixel = (color.r << 16) | (color.g << 8) | color.b;
    for (int i = 0; i < total; i++) {
        renderer->pixels[i] = pixel;
    }
}

void vg_renderer_present(VgRenderer* renderer) {
    if (!renderer || !renderer->window) return;
    #ifdef _WIN32
        BitBlt(renderer->window->hdc, 0, 0,
               renderer->window->width, renderer->window->height,
               renderer->mem_dc, 0, 0, SRCCOPY);
    #endif
}

static inline void vg_put_pixel(VgRenderer* r, int x, int y, VlColor color) {
    if (x < 0 || y < 0 || x >= r->window->width || y >= r->window->height) return;
    r->pixels[y * r->window->width + x] = (color.r << 16) | (color.g << 8) | color.b;
}

void vg_renderer_draw_rect(VgRenderer* renderer, VgRect rect, VlColor color) {
    if (!renderer || !renderer->pixels) return;
    int x0 = (int)(rect.x - renderer->camera_x);
    int y0 = (int)(rect.y - renderer->camera_y);
    int x1 = x0 + (int)rect.w;
    int y1 = y0 + (int)rect.h;

    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            vg_put_pixel(renderer, x, y, color);
        }
    }
}

void vg_renderer_draw_rect_outline(VgRenderer* renderer, VgRect rect, VlColor color, float thickness) {
    if (!renderer) return;
    int t = (int)thickness;
    vg_renderer_draw_rect(renderer, vg_rect(rect.x, rect.y, rect.w, t), color);
    vg_renderer_draw_rect(renderer, vg_rect(rect.x, rect.y + rect.h - t, rect.w, t), color);
    vg_renderer_draw_rect(renderer, vg_rect(rect.x, rect.y, t, rect.h), color);
    vg_renderer_draw_rect(renderer, vg_rect(rect.x + rect.w - t, rect.y, t, rect.h), color);
}

void vg_renderer_draw_circle(VgRenderer* renderer, double cx, double cy, double radius, VlColor color) {
    if (!renderer || !renderer->pixels) return;
    int r2 = (int)(radius * radius);
    int icx = (int)(cx - renderer->camera_x);
    int icy = (int)(cy - renderer->camera_y);

    for (int y = (int)-radius; y <= (int)radius; y++) {
        for (int x = (int)-radius; x <= (int)radius; x++) {
            if (x * x + y * y <= r2) {
                vg_put_pixel(renderer, icx + x, icy + y, color);
            }
        }
    }
}

void vg_renderer_draw_line(VgRenderer* renderer, double x1, double y1, double x2, double y2, VlColor color, float thickness) {
    if (!renderer || !renderer->pixels) return;

    int ix1 = (int)(x1 - renderer->camera_x);
    int iy1 = (int)(y1 - renderer->camera_y);
    int ix2 = (int)(x2 - renderer->camera_x);
    int iy2 = (int)(y2 - renderer->camera_y);

    int dx = abs(ix2 - ix1);
    int dy = abs(iy2 - iy1);
    int sx = ix1 < ix2 ? 1 : -1;
    int sy = iy1 < iy2 ? 1 : -1;
    int err = dx - dy;

    while (1) {
        int t = (int)(thickness / 2);
        for (int ty = -t; ty <= t; ty++) {
            for (int tx = -t; tx <= t; tx++) {
                vg_put_pixel(renderer, ix1 + tx, iy1 + ty, color);
            }
        }

        if (ix1 == ix2 && iy1 == iy2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; ix1 += sx; }
        if (e2 < dx) { err += dx; iy1 += sy; }
    }
}

void vg_renderer_draw_sprite(VgRenderer* renderer, VgSprite* sprite) {
    if (!renderer || !sprite || !sprite->visible || !sprite->texture) return;

    VgTexture* tex = sprite->texture;
    int sx = (int)(sprite->position.x - renderer->camera_x);
    int sy = (int)(sprite->position.y - renderer->camera_y);
    int sw = (int)(tex->width * sprite->scale.x);
    int sh = (int)(tex->height * sprite->scale.y);

    for (int y = 0; y < sh; y++) {
        for (int x = 0; x < sw; x++) {
            int tx = (int)((double)x / sprite->scale.x);
            int ty = (int)((double)y / sprite->scale.y);
            if (sprite->flip_h) tx = tex->width - 1 - tx;
            if (sprite->flip_v) ty = tex->height - 1 - ty;

            if (tx >= 0 && tx < tex->width && ty >= 0 && ty < tex->height) {
                uint32_t pixel = tex->pixels[ty * tex->width + tx];
                uint8_t a = (pixel >> 24) & 0xFF;
                if (a > 128) {
                    VlColor c;
                    c.r = (pixel >> 16) & 0xFF;
                    c.g = (pixel >> 8) & 0xFF;
                    c.b = pixel & 0xFF;
                    c.a = a;
                    vg_put_pixel(renderer, sx + x, sy + y, c);
                }
            }
        }
    }
}

void vg_renderer_draw_text(VgRenderer* renderer, const char* text, double x, double y, VlColor color) {
    if (!renderer || !text) return;
    // Simple bitmap text rendering placeholder
    // Full font rendering requires FreeType integration
    #ifdef _WIN32
    {
        int ix = (int)(x - renderer->camera_x);
        int iy = (int)(y - renderer->camera_y);
        SetBkMode(renderer->mem_dc, TRANSPARENT);
        SetTextColor(renderer->mem_dc, RGB(color.r, color.g, color.b));
        TextOutA(renderer->mem_dc, ix, iy, text, (int)strlen(text));
    }
    #endif
}

// ─── Texture ───

VgTexture* vg_texture_load(VgRenderer* renderer, const char* path) {
    (void)renderer;
    VgTexture* tex = (VgTexture*)calloc(1, sizeof(VgTexture));

    // Simple BMP loader for basic texture support
    FILE* f = fopen(path, "rb");
    if (!f) {
        vl_log_warn("Failed to load texture: %s", path);
        tex->width = 32;
        tex->height = 32;
        tex->pixels = (uint32_t*)calloc(32 * 32, sizeof(uint32_t));
        for (int i = 0; i < 32 * 32; i++) {
            tex->pixels[i] = 0xFFFF00FF;
        }
        return tex;
    }

    uint8_t header[54];
    fread(header, 1, 54, f);

    if (header[0] == 'B' && header[1] == 'M') {
        tex->width = *(int*)&header[18];
        tex->height = abs(*(int*)&header[22]);
        int bpp = *(short*)&header[28];
        int offset = *(int*)&header[10];

        fseek(f, offset, SEEK_SET);
        tex->pixels = (uint32_t*)calloc(tex->width * tex->height, sizeof(uint32_t));

        if (bpp == 24) {
            int row_size = ((tex->width * 3 + 3) / 4) * 4;
            uint8_t* row = (uint8_t*)malloc(row_size);
            for (int y = tex->height - 1; y >= 0; y--) {
                fread(row, 1, row_size, f);
                for (int x = 0; x < tex->width; x++) {
                    uint8_t b = row[x * 3];
                    uint8_t g = row[x * 3 + 1];
                    uint8_t r = row[x * 3 + 2];
                    tex->pixels[y * tex->width + x] = (255u << 24) | (r << 16) | (g << 8) | b;
                }
            }
            free(row);
        } else if (bpp == 32) {
            fread(tex->pixels, sizeof(uint32_t), tex->width * tex->height, f);
        }
    }

    fclose(f);
    return tex;
}

void vg_texture_destroy(VgTexture* texture) {
    if (!texture) return;
    if (texture->pixels) free(texture->pixels);
    free(texture);
}

int vg_texture_width(VgTexture* texture) { return texture ? texture->width : 0; }
int vg_texture_height(VgTexture* texture) { return texture ? texture->height : 0; }

// ─── Sprite ───

VgSprite* vg_sprite_create(VgTexture* texture) {
    VgSprite* s = (VgSprite*)calloc(1, sizeof(VgSprite));
    s->texture = texture;
    s->position = vl_vec2(0, 0);
    s->scale = vl_vec2(1, 1);
    s->rotation = 0;
    s->color = vl_color(255, 255, 255, 255);
    s->visible = 1;
    s->flip_h = 0;
    s->flip_v = 0;
    return s;
}

VgSprite* vg_sprite_create_from_file(VgRenderer* renderer, const char* path) {
    VgTexture* tex = vg_texture_load(renderer, path);
    return vg_sprite_create(tex);
}

void vg_sprite_destroy(VgSprite* sprite) {
    if (sprite) free(sprite);
}

void vg_sprite_set_position(VgSprite* sprite, double x, double y) {
    if (sprite) sprite->position = vl_vec2(x, y);
}

void vg_sprite_set_scale(VgSprite* sprite, double sx, double sy) {
    if (sprite) sprite->scale = vl_vec2(sx, sy);
}

void vg_sprite_set_rotation(VgSprite* sprite, double angle) {
    if (sprite) sprite->rotation = angle;
}

void vg_sprite_set_color(VgSprite* sprite, VlColor color) {
    if (sprite) sprite->color = color;
}

void vg_sprite_set_visible(VgSprite* sprite, int visible) {
    if (sprite) sprite->visible = visible;
}

void vg_sprite_flip_x(VgSprite* sprite) {
    if (sprite) sprite->flip_h = !sprite->flip_h;
}

void vg_sprite_flip_y(VgSprite* sprite) {
    if (sprite) sprite->flip_v = !sprite->flip_v;
}

VlVector2 vg_sprite_position(VgSprite* sprite) {
    if (sprite) return sprite->position;
    return vl_vec2(0, 0);
}

// ─── Input ───

int vg_key_pressed(int key_code) {
    if (key_code < 0 || key_code >= VG_MAX_KEYS) return 0;
    return key_state[key_code];
}

int vg_key_just_pressed(int key_code) {
    if (key_code < 0 || key_code >= VG_MAX_KEYS) return 0;
    return key_state[key_code] && !key_prev[key_code];
}

int vg_key_released(int key_code) {
    if (key_code < 0 || key_code >= VG_MAX_KEYS) return 0;
    return !key_state[key_code] && key_prev[key_code];
}

VlVector2 vg_mouse_position() {
    return vl_vec2((double)mouse_x, (double)mouse_y);
}

int vg_mouse_pressed(VgMouseButton button) {
    return mouse_state[button];
}

int vg_mouse_just_pressed(VgMouseButton button) {
    return mouse_state[button] && !mouse_prev[button];
}

void vg_input_update() {
    memcpy(key_prev, key_state, sizeof(key_prev));
    memcpy(mouse_prev, mouse_state, sizeof(mouse_prev));
}

// ─── Camera ───

void vg_camera_set_position(VgRenderer* renderer, double x, double y) {
    if (renderer) { renderer->camera_x = x; renderer->camera_y = y; }
}

void vg_camera_set_zoom(VgRenderer* renderer, double zoom) {
    if (renderer) renderer->camera_zoom = zoom;
}
