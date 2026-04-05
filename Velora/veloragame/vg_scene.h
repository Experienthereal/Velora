#ifndef VELORAGAME_SCENE_H
#define VELORAGAME_SCENE_H

#include "veloragame.h"
#include "vg_physics.h"
#include "vg_ui.h"
#include "vg_sound.h"
#include <stdlib.h>
#include <string.h>

// ─── Scene System ───
// Scenes are equivalent to game states or levels.
// Each scene owns its own entities, physics world, and UI context.

typedef struct VgScene VgScene;

typedef void (*VgSceneLoad)   (VgScene* scene);
typedef void (*VgSceneUnload) (VgScene* scene);
typedef void (*VgSceneUpdate) (VgScene* scene, double delta);
typedef void (*VgSceneRender) (VgScene* scene, VgRenderer* renderer);

#define VG_MAX_ENTITIES 2048
#define VG_MAX_SCENE_NAME 64

// ─── Entity ───

typedef struct {
    char name[64];
    VlVector2 position;
    VlVector2 scale;
    double rotation;
    int active;
    VgSprite* sprite;
    VgRigidBody* body;
    void* component_data;
    uint32_t id;
} VgEntity;

// ─── Scene ───

struct VgScene {
    char name[VG_MAX_SCENE_NAME];
    VgEntity* entities[VG_MAX_ENTITIES];
    int entity_count;
    VgPhysicsWorld* physics;
    VgUIContext* ui;
    void* user_data;

    VgSceneLoad   on_load;
    VgSceneUnload on_unload;
    VgSceneUpdate on_update;
    VgSceneRender on_render;
};

// ─── Scene Manager ───

#define VG_MAX_SCENES 32

typedef struct {
    VgScene* scenes[VG_MAX_SCENES];
    int scene_count;
    VgScene* current;
    VgScene* pending_next;
    VgRenderer* renderer;
    VgWindow* window;
    VlTimer timer;
} VgSceneManager;

static inline VgScene* vg_scene_create(const char* name) {
    VgScene* scene = (VgScene*)calloc(1, sizeof(VgScene));
    strncpy(scene->name, name, VG_MAX_SCENE_NAME - 1);
    scene->physics = vg_physics_world_create(0, 980.0);
    scene->ui = vg_ui_create();
    return scene;
}

static inline void vg_scene_destroy(VgScene* scene) {
    if (!scene) return;
    if (scene->on_unload) scene->on_unload(scene);
    vg_physics_world_destroy(scene->physics);
    vg_ui_destroy(scene->ui);
    for (int i = 0; i < scene->entity_count; i++) {
        free(scene->entities[i]);
    }
    free(scene);
}

static inline VgEntity* vg_scene_add_entity(VgScene* scene, const char* name) {
    if (scene->entity_count >= VG_MAX_ENTITIES) return NULL;
    VgEntity* e = (VgEntity*)calloc(1, sizeof(VgEntity));
    strncpy(e->name, name, sizeof(e->name) - 1);
    e->active = 1;
    e->scale = vl_vec2(1, 1);
    e->id = (uint32_t)scene->entity_count;
    scene->entities[scene->entity_count++] = e;
    return e;
}

static inline VgEntity* vg_scene_find_entity(VgScene* scene, const char* name) {
    for (int i = 0; i < scene->entity_count; i++) {
        if (strcmp(scene->entities[i]->name, name) == 0) {
            return scene->entities[i];
        }
    }
    return NULL;
}

static inline void vg_scene_remove_entity(VgScene* scene, VgEntity* entity) {
    for (int i = 0; i < scene->entity_count; i++) {
        if (scene->entities[i] == entity) {
            if (entity->body) vg_physics_world_remove(scene->physics, entity->body);
            free(entity);
            scene->entities[i] = scene->entities[--scene->entity_count];
            return;
        }
    }
}

// ─── Scene Manager ───

static inline VgSceneManager* vg_scene_manager_create(VgWindow* window, VgRenderer* renderer) {
    VgSceneManager* mgr = (VgSceneManager*)calloc(1, sizeof(VgSceneManager));
    mgr->window = window;
    mgr->renderer = renderer;
    mgr->timer = vl_timer_create();
    return mgr;
}

static inline void vg_scene_manager_add(VgSceneManager* mgr, VgScene* scene) {
    if (mgr->scene_count < VG_MAX_SCENES) {
        mgr->scenes[mgr->scene_count++] = scene;
    }
}

static inline void vg_scene_manager_switch(VgSceneManager* mgr, const char* name) {
    for (int i = 0; i < mgr->scene_count; i++) {
        if (strcmp(mgr->scenes[i]->name, name) == 0) {
            mgr->pending_next = mgr->scenes[i];
            return;
        }
    }
}

static inline void vg_scene_manager_run(VgSceneManager* mgr, VgScene* start_scene) {
    mgr->current = start_scene;
    if (mgr->current && mgr->current->on_load) {
        mgr->current->on_load(mgr->current);
    }

    while (vg_window_is_open(mgr->window)) {
        vl_timer_tick(&mgr->timer);
        double delta = vl_timer_delta(&mgr->timer);
        if (delta > 0.05) delta = 0.05;

        // Handle pending scene switch
        if (mgr->pending_next && mgr->pending_next != mgr->current) {
            if (mgr->current && mgr->current->on_unload) {
                mgr->current->on_unload(mgr->current);
            }
            mgr->current = mgr->pending_next;
            mgr->pending_next = NULL;
            if (mgr->current->on_load) {
                mgr->current->on_load(mgr->current);
            }
        }

        // Poll events
        VgEvent event;
        while (vg_window_poll_event(mgr->window, &event)) {
            if (event.type == VG_EVENT_QUIT) {
                vg_window_close(mgr->window);
            }
        }

        VlVector2 mouse_pos = vg_mouse_position();
        vg_input_update();

        if (mgr->current) {
            // Update UI
            vg_ui_update(mgr->current->ui,
                (int)mouse_pos.x, (int)mouse_pos.y,
                vg_mouse_pressed(VG_MOUSE_LEFT));

            // Update physics
            vg_physics_step(mgr->current->physics, delta);

            // Sync entity positions with physics bodies
            for (int i = 0; i < mgr->current->entity_count; i++) {
                VgEntity* e = mgr->current->entities[i];
                if (e->body && e->sprite) {
                    vg_sprite_set_position(e->sprite,
                        e->body->position.x, e->body->position.y);
                    e->position = e->body->position;
                }
            }

            // Custom update
            if (mgr->current->on_update) {
                mgr->current->on_update(mgr->current, delta);
            }

            // Render
            if (mgr->current->on_render) {
                mgr->current->on_render(mgr->current, mgr->renderer);
            }

            // Render UI on top
            vg_ui_render(mgr->renderer, mgr->current->ui);
        }

        vg_renderer_present(mgr->renderer);
    }
}

static inline void vg_scene_manager_destroy(VgSceneManager* mgr) {
    if (!mgr) return;
    for (int i = 0; i < mgr->scene_count; i++) {
        vg_scene_destroy(mgr->scenes[i]);
    }
    free(mgr);
}

#endif
