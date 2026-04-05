#ifndef VELORAGAME_PHYSICS_H
#define VELORAGAME_PHYSICS_H

#include "../stdlib/vl_math.h"
#include <stdlib.h>
#include <string.h>

// ─── Collision Shapes ───

typedef enum {
    VG_COLLIDER_BOX,
    VG_COLLIDER_CIRCLE,
    VG_COLLIDER_NONE
} VgColliderType;

typedef struct {
    VgColliderType type;
    VlVector2 position;
    double width;
    double height;
    double radius;
    int is_trigger;
    int is_static;
    int layer;
    void* user_data;
} VgCollider;

typedef struct {
    int collided;
    VlVector2 normal;
    double depth;
    VgCollider* a;
    VgCollider* b;
} VgCollision;

// ─── Rigid Body ───

typedef struct {
    VlVector2 position;
    VlVector2 velocity;
    VlVector2 acceleration;
    double mass;
    double friction;
    double restitution;
    double gravity_scale;
    int is_static;
    int on_ground;
    VgCollider collider;
} VgRigidBody;

// ─── Physics World ───

#define VG_MAX_BODIES 1024

typedef struct {
    VgRigidBody* bodies[VG_MAX_BODIES];
    int count;
    VlVector2 gravity;
    double fixed_step;
    double accumulator;
} VgPhysicsWorld;

// ─── Constructors ───

static inline VgCollider vg_collider_box(double x, double y, double w, double h) {
    VgCollider c = {0};
    c.type = VG_COLLIDER_BOX;
    c.position = vl_vec2(x, y);
    c.width = w;
    c.height = h;
    return c;
}

static inline VgCollider vg_collider_circle(double x, double y, double radius) {
    VgCollider c = {0};
    c.type = VG_COLLIDER_CIRCLE;
    c.position = vl_vec2(x, y);
    c.radius = radius;
    return c;
}

static inline VgRigidBody* vg_rigidbody_create(double x, double y, double mass) {
    VgRigidBody* body = (VgRigidBody*)calloc(1, sizeof(VgRigidBody));
    body->position = vl_vec2(x, y);
    body->velocity = vl_vec2(0, 0);
    body->acceleration = vl_vec2(0, 0);
    body->mass = mass;
    body->friction = 0.1;
    body->restitution = 0.3;
    body->gravity_scale = 1.0;
    body->is_static = 0;
    body->on_ground = 0;
    return body;
}

static inline void vg_rigidbody_destroy(VgRigidBody* body) {
    free(body);
}

static inline void vg_rigidbody_apply_force(VgRigidBody* body, VlVector2 force) {
    if (body->is_static || body->mass <= 0) return;
    body->acceleration.x += force.x / body->mass;
    body->acceleration.y += force.y / body->mass;
}

static inline void vg_rigidbody_apply_impulse(VgRigidBody* body, VlVector2 impulse) {
    if (body->is_static) return;
    body->velocity.x += impulse.x;
    body->velocity.y += impulse.y;
}

// ─── Collision Detection ───

static inline int vg_aabb_collide(VgCollider* a, VgCollider* b) {
    double ax = a->position.x, ay = a->position.y;
    double bx = b->position.x, by = b->position.y;

    if (a->type == VG_COLLIDER_BOX && b->type == VG_COLLIDER_BOX) {
        return ax < bx + b->width  && ax + a->width  > bx &&
               ay < by + b->height && ay + a->height > by;
    }

    if (a->type == VG_COLLIDER_CIRCLE && b->type == VG_COLLIDER_CIRCLE) {
        double dx = ax - bx;
        double dy = ay - by;
        double dist_sq = dx * dx + dy * dy;
        double radii = a->radius + b->radius;
        return dist_sq <= radii * radii;
    }

    if (a->type == VG_COLLIDER_BOX && b->type == VG_COLLIDER_CIRCLE) {
        double closest_x = vl_clamp(bx, ax, ax + a->width);
        double closest_y = vl_clamp(by, ay, ay + a->height);
        double dx = bx - closest_x;
        double dy = by - closest_y;
        return (dx * dx + dy * dy) <= (b->radius * b->radius);
    }

    if (a->type == VG_COLLIDER_CIRCLE && b->type == VG_COLLIDER_BOX) {
        double closest_x = vl_clamp(ax, bx, bx + b->width);
        double closest_y = vl_clamp(ay, by, by + b->height);
        double dx = ax - closest_x;
        double dy = ay - closest_y;
        return (dx * dx + dy * dy) <= (a->radius * a->radius);
    }

    return 0;
}

static inline VgCollision vg_check_collision(VgCollider* a, VgCollider* b) {
    VgCollision col = {0};
    col.a = a;
    col.b = b;
    col.collided = vg_aabb_collide(a, b);

    if (col.collided && a->type == VG_COLLIDER_BOX && b->type == VG_COLLIDER_BOX) {
        double overlap_x = (a->position.x + a->width / 2) - (b->position.x + b->width / 2);
        double overlap_y = (a->position.y + a->height / 2) - (b->position.y + b->height / 2);
        double half_w = (a->width + b->width) / 2;
        double half_h = (a->height + b->height) / 2;

        if (overlap_x < 0) col.normal.x = -1; else col.normal.x = 1;
        if (overlap_y < 0) col.normal.y = -1; else col.normal.y = 1;

        col.depth = half_w - (overlap_x < 0 ? -overlap_x : overlap_x);
        double depth_y = half_h - (overlap_y < 0 ? -overlap_y : overlap_y);

        if (depth_y < col.depth) {
            col.depth = depth_y;
            col.normal.x = 0;
            col.normal.y = overlap_y < 0 ? -1 : 1;
        } else {
            col.normal.y = 0;
        }
    }

    return col;
}

// ─── Physics World ───

static inline VgPhysicsWorld* vg_physics_world_create(double gx, double gy) {
    VgPhysicsWorld* world = (VgPhysicsWorld*)calloc(1, sizeof(VgPhysicsWorld));
    world->gravity = vl_vec2(gx, gy);
    world->fixed_step = 1.0 / 60.0;
    world->accumulator = 0;
    return world;
}

static inline void vg_physics_world_add(VgPhysicsWorld* world, VgRigidBody* body) {
    if (world->count < VG_MAX_BODIES) {
        world->bodies[world->count++] = body;
    }
}

static inline void vg_physics_world_remove(VgPhysicsWorld* world, VgRigidBody* body) {
    for (int i = 0; i < world->count; i++) {
        if (world->bodies[i] == body) {
            world->bodies[i] = world->bodies[--world->count];
            return;
        }
    }
}

static inline void vg_physics_step(VgPhysicsWorld* world, double delta) {
    world->accumulator += delta;

    while (world->accumulator >= world->fixed_step) {
        double dt = world->fixed_step;

        for (int i = 0; i < world->count; i++) {
            VgRigidBody* b = world->bodies[i];
            if (b->is_static) continue;

            // Apply gravity
            b->acceleration.x += world->gravity.x * b->gravity_scale;
            b->acceleration.y += world->gravity.y * b->gravity_scale;

            // Integrate velocity
            b->velocity.x += b->acceleration.x * dt;
            b->velocity.y += b->acceleration.y * dt;

            // Apply friction
            b->velocity.x *= (1.0 - b->friction * dt);

            // Integrate position
            b->position.x += b->velocity.x * dt;
            b->position.y += b->velocity.y * dt;

            // Reset acceleration (forces are re-applied each step)
            b->acceleration.x = 0;
            b->acceleration.y = 0;

            // Update collider position
            b->collider.position = b->position;
        }

        // Broad-phase collision resolution
        for (int i = 0; i < world->count; i++) {
            for (int j = i + 1; j < world->count; j++) {
                VgRigidBody* a = world->bodies[i];
                VgRigidBody* b = world->bodies[j];

                if (a->collider.type == VG_COLLIDER_NONE) continue;
                if (b->collider.type == VG_COLLIDER_NONE) continue;

                VgCollision col = vg_check_collision(&a->collider, &b->collider);
                if (!col.collided || col.is_trigger) continue;

                // Positional correction
                double correction = col.depth * 0.8;
                if (!a->is_static) {
                    a->position.x -= col.normal.x * correction * 0.5;
                    a->position.y -= col.normal.y * correction * 0.5;
                }
                if (!b->is_static) {
                    b->position.x += col.normal.x * correction * 0.5;
                    b->position.y += col.normal.y * correction * 0.5;
                }

                // Velocity resolution
                double rel_vx = b->velocity.x - a->velocity.x;
                double rel_vy = b->velocity.y - a->velocity.y;
                double dot = rel_vx * col.normal.x + rel_vy * col.normal.y;

                if (dot > 0) continue;

                double e = (a->restitution + b->restitution) * 0.5;
                double j = -(1 + e) * dot;
                double total_mass = (a->is_static ? 0 : 1.0 / a->mass) + (b->is_static ? 0 : 1.0 / b->mass);
                if (total_mass > 0) j /= total_mass;

                if (!a->is_static) {
                    a->velocity.x -= (j / a->mass) * col.normal.x;
                    a->velocity.y -= (j / a->mass) * col.normal.y;
                }
                if (!b->is_static) {
                    b->velocity.x += (j / b->mass) * col.normal.x;
                    b->velocity.y += (j / b->mass) * col.normal.y;
                }

                // Ground detection (collision normal pointing up)
                if (col.normal.y < -0.5) a->on_ground = 1;
                if (col.normal.y >  0.5) b->on_ground = 1;
            }
        }

        world->accumulator -= world->fixed_step;
    }
}

static inline void vg_physics_world_destroy(VgPhysicsWorld* world) {
    free(world);
}

#endif
