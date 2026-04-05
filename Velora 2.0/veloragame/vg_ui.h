#ifndef VELORAGAME_UI_H
#define VELORAGAME_UI_H

#include "veloragame.h"
#include <string.h>
#include <stdlib.h>

// ─── UI Element Types ───

typedef enum {
    VG_UI_LABEL,
    VG_UI_BUTTON,
    VG_UI_PANEL,
    VG_UI_SLIDER,
    VG_UI_CHECKBOX,
    VG_UI_TEXTBOX,
    VG_UI_IMAGE,
    VG_UI_PROGRESSBAR
} VgUIType;

typedef enum {
    VG_UI_ALIGN_LEFT,
    VG_UI_ALIGN_CENTER,
    VG_UI_ALIGN_RIGHT
} VgUIAlign;

// ─── UI Style ───

typedef struct {
    VlColor background;
    VlColor foreground;
    VlColor border;
    VlColor hover;
    VlColor active;
    float border_width;
    float corner_radius;
    int font_size;
    VgUIAlign text_align;
} VgUIStyle;

static inline VgUIStyle vg_ui_default_style() {
    VgUIStyle s;
    s.background   = vl_color(40, 40, 50, 255);
    s.foreground   = vl_color(220, 220, 235, 255);
    s.border       = vl_color(80, 80, 100, 255);
    s.hover        = vl_color(60, 60, 80, 255);
    s.active       = vl_color(80, 120, 200, 255);
    s.border_width = 1.0f;
    s.corner_radius = 4.0f;
    s.font_size    = 14;
    s.text_align   = VG_UI_ALIGN_CENTER;
    return s;
}

// ─── UI Element ───

typedef struct VgUIElement VgUIElement;

typedef void (*VgUICallback)(VgUIElement* element, void* user_data);

struct VgUIElement {
    VgUIType type;
    VgRect rect;
    VgUIStyle style;
    char text[256];
    char id[64];
    int visible;
    int enabled;
    int hovered;
    int pressed;
    int focused;

    // State for interactive elements
    double value;
    double min_value;
    double max_value;
    int checked;

    // Callbacks
    VgUICallback on_click;
    VgUICallback on_hover;
    VgUICallback on_change;
    void* user_data;

    // Children (for Panel)
    VgUIElement** children;
    int child_count;
    int child_capacity;
};

// ─── UI Context ───

#define VG_UI_MAX_ELEMENTS 256

typedef struct {
    VgUIElement* elements[VG_UI_MAX_ELEMENTS];
    int count;
    int mouse_x;
    int mouse_y;
    int mouse_down;
    int mouse_just_clicked;
    VgUIElement* focused_element;
} VgUIContext;

static inline VgUIContext* vg_ui_create() {
    return (VgUIContext*)calloc(1, sizeof(VgUIContext));
}

static inline void vg_ui_destroy(VgUIContext* ctx) {
    if (!ctx) return;
    for (int i = 0; i < ctx->count; i++) {
        if (ctx->elements[i]->children) free(ctx->elements[i]->children);
        free(ctx->elements[i]);
    }
    free(ctx);
}

// ─── Element Constructors ───

static inline VgUIElement* vg_ui_element_create(VgUIType type, const char* id,
                                                  double x, double y, double w, double h) {
    VgUIElement* el = (VgUIElement*)calloc(1, sizeof(VgUIElement));
    el->type = type;
    el->rect = vg_rect(x, y, w, h);
    el->style = vg_ui_default_style();
    strncpy(el->id, id, sizeof(el->id) - 1);
    el->visible = 1;
    el->enabled = 1;
    el->min_value = 0;
    el->max_value = 1;
    el->value = 0;
    return el;
}

static inline VgUIElement* vg_ui_label(VgUIContext* ctx, const char* id,
                                        const char* text, double x, double y, double w, double h) {
    VgUIElement* el = vg_ui_element_create(VG_UI_LABEL, id, x, y, w, h);
    strncpy(el->text, text, sizeof(el->text) - 1);
    el->style.background = vl_color(0, 0, 0, 0);
    if (ctx->count < VG_UI_MAX_ELEMENTS) ctx->elements[ctx->count++] = el;
    return el;
}

static inline VgUIElement* vg_ui_button(VgUIContext* ctx, const char* id,
                                          const char* text, double x, double y, double w, double h) {
    VgUIElement* el = vg_ui_element_create(VG_UI_BUTTON, id, x, y, w, h);
    strncpy(el->text, text, sizeof(el->text) - 1);
    if (ctx->count < VG_UI_MAX_ELEMENTS) ctx->elements[ctx->count++] = el;
    return el;
}

static inline VgUIElement* vg_ui_panel(VgUIContext* ctx, const char* id,
                                         double x, double y, double w, double h) {
    VgUIElement* el = vg_ui_element_create(VG_UI_PANEL, id, x, y, w, h);
    el->children = (VgUIElement**)calloc(32, sizeof(VgUIElement*));
    el->child_capacity = 32;
    if (ctx->count < VG_UI_MAX_ELEMENTS) ctx->elements[ctx->count++] = el;
    return el;
}

static inline VgUIElement* vg_ui_slider(VgUIContext* ctx, const char* id,
                                          double x, double y, double w, double h,
                                          double min_val, double max_val, double initial) {
    VgUIElement* el = vg_ui_element_create(VG_UI_SLIDER, id, x, y, w, h);
    el->min_value = min_val;
    el->max_value = max_val;
    el->value = initial;
    if (ctx->count < VG_UI_MAX_ELEMENTS) ctx->elements[ctx->count++] = el;
    return el;
}

static inline VgUIElement* vg_ui_checkbox(VgUIContext* ctx, const char* id,
                                            const char* text, double x, double y,
                                            int initial_checked) {
    VgUIElement* el = vg_ui_element_create(VG_UI_CHECKBOX, id, x, y, 24, 24);
    strncpy(el->text, text, sizeof(el->text) - 1);
    el->checked = initial_checked;
    if (ctx->count < VG_UI_MAX_ELEMENTS) ctx->elements[ctx->count++] = el;
    return el;
}

static inline VgUIElement* vg_ui_progressbar(VgUIContext* ctx, const char* id,
                                               double x, double y, double w, double h,
                                               double value) {
    VgUIElement* el = vg_ui_element_create(VG_UI_PROGRESSBAR, id, x, y, w, h);
    el->value = vl_clamp(value, 0, 1);
    if (ctx->count < VG_UI_MAX_ELEMENTS) ctx->elements[ctx->count++] = el;
    return el;
}

// ─── UI Update (Input Processing) ───

static inline void vg_ui_update(VgUIContext* ctx, int mx, int my, int mouse_down) {
    int was_down = ctx->mouse_down;
    ctx->mouse_x = mx;
    ctx->mouse_y = my;
    ctx->mouse_down = mouse_down;
    ctx->mouse_just_clicked = (!mouse_down && was_down);

    for (int i = 0; i < ctx->count; i++) {
        VgUIElement* el = ctx->elements[i];
        if (!el->visible || !el->enabled) continue;

        el->hovered = vl_point_in_rect(mx, my, el->rect.x, el->rect.y, el->rect.w, el->rect.h);

        if (el->hovered) {
            if (el->on_hover) el->on_hover(el, el->user_data);

            if (ctx->mouse_just_clicked) {
                el->pressed = 1;
                if (el->type == VG_UI_CHECKBOX) {
                    el->checked = !el->checked;
                }
                if (el->on_click) el->on_click(el, el->user_data);
            } else {
                el->pressed = 0;
            }

            if (el->type == VG_UI_SLIDER && mouse_down) {
                double ratio = (mx - el->rect.x) / el->rect.w;
                ratio = vl_clamp(ratio, 0, 1);
                el->value = el->min_value + ratio * (el->max_value - el->min_value);
                if (el->on_change) el->on_change(el, el->user_data);
            }
        } else {
            el->hovered = 0;
            el->pressed = 0;
        }
    }
}

// ─── UI Render ───

static inline void vg_ui_render_element(VgRenderer* renderer, VgUIElement* el) {
    if (!el->visible) return;

    VlColor bg = el->hovered ? el->style.hover : el->style.background;
    if (el->pressed) bg = el->style.active;

    switch (el->type) {
        case VG_UI_LABEL:
            vg_renderer_draw_text(renderer, el->text,
                el->rect.x + 4, el->rect.y + el->rect.h / 2 - 8,
                el->style.foreground);
            break;

        case VG_UI_BUTTON:
            vg_renderer_draw_rect(renderer, el->rect, bg);
            vg_renderer_draw_rect_outline(renderer, el->rect, el->style.border, el->style.border_width);
            vg_renderer_draw_text(renderer, el->text,
                el->rect.x + el->rect.w / 2 - (int)(strlen(el->text) * 4),
                el->rect.y + el->rect.h / 2 - 8,
                el->style.foreground);
            break;

        case VG_UI_PANEL:
            vg_renderer_draw_rect(renderer, el->rect, el->style.background);
            vg_renderer_draw_rect_outline(renderer, el->rect, el->style.border, el->style.border_width);
            for (int i = 0; i < el->child_count; i++) {
                vg_ui_render_element(renderer, el->children[i]);
            }
            break;

        case VG_UI_SLIDER: {
            VgRect track = vg_rect(el->rect.x, el->rect.y + el->rect.h * 0.35,
                                   el->rect.w, el->rect.h * 0.3);
            vg_renderer_draw_rect(renderer, track, el->style.border);

            double ratio = (el->value - el->min_value) / (el->max_value - el->min_value);
            double knob_x = el->rect.x + ratio * el->rect.w - 8;
            VgRect knob = vg_rect(knob_x, el->rect.y, 16, el->rect.h);
            vg_renderer_draw_rect(renderer, knob, el->style.active);
            break;
        }

        case VG_UI_CHECKBOX: {
            VgRect box = vg_rect(el->rect.x, el->rect.y, 20, 20);
            vg_renderer_draw_rect(renderer, box, bg);
            vg_renderer_draw_rect_outline(renderer, box, el->style.border, 1.5f);
            if (el->checked) {
                VgRect check = vg_rect(el->rect.x + 4, el->rect.y + 4, 12, 12);
                vg_renderer_draw_rect(renderer, check, el->style.active);
            }
            vg_renderer_draw_text(renderer, el->text,
                el->rect.x + 28, el->rect.y + 2, el->style.foreground);
            break;
        }

        case VG_UI_PROGRESSBAR: {
            vg_renderer_draw_rect(renderer, el->rect, el->style.background);
            VgRect fill = vg_rect(el->rect.x, el->rect.y,
                                  el->rect.w * vl_clamp(el->value, 0, 1), el->rect.h);
            vg_renderer_draw_rect(renderer, fill, el->style.active);
            vg_renderer_draw_rect_outline(renderer, el->rect, el->style.border, 1.0f);
            break;
        }

        default:
            break;
    }
}

static inline void vg_ui_render(VgRenderer* renderer, VgUIContext* ctx) {
    for (int i = 0; i < ctx->count; i++) {
        vg_ui_render_element(renderer, ctx->elements[i]);
    }
}

// ─── UI Helpers ───

static inline VgUIElement* vg_ui_find(VgUIContext* ctx, const char* id) {
    for (int i = 0; i < ctx->count; i++) {
        if (strcmp(ctx->elements[i]->id, id) == 0) {
            return ctx->elements[i];
        }
    }
    return NULL;
}

static inline int vg_ui_button_clicked(VgUIContext* ctx, const char* id) {
    VgUIElement* el = vg_ui_find(ctx, id);
    return el && el->pressed;
}

static inline double vg_ui_slider_value(VgUIContext* ctx, const char* id) {
    VgUIElement* el = vg_ui_find(ctx, id);
    return el ? el->value : 0;
}

static inline int vg_ui_checkbox_checked(VgUIContext* ctx, const char* id) {
    VgUIElement* el = vg_ui_find(ctx, id);
    return el ? el->checked : 0;
}

static inline void vg_ui_set_text(VgUIContext* ctx, const char* id, const char* text) {
    VgUIElement* el = vg_ui_find(ctx, id);
    if (el) strncpy(el->text, text, sizeof(el->text) - 1);
}

static inline void vg_ui_set_progress(VgUIContext* ctx, const char* id, double value) {
    VgUIElement* el = vg_ui_find(ctx, id);
    if (el) el->value = vl_clamp(value, 0, 1);
}

#endif
