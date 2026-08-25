#ifndef _CAMERA_H
#define _CAMERA_H

#include "macros.h"
#include "fajter.h" 

typedef struct camera_t
{
    vec2f_t position;   // current center, world space
    float   zoom;       // current zoom, 1.0 = normal
    
    SDL_Rect view;
    vec2f_t shake_offset;
    float   shake_time;   // remaining shake duration
    float   shake_strength;
} camera_t;

#define CAMERA_MIN_ZOOM 1.4f
#define CAMERA_MAX_ZOOM 1.9f
#define CAMERA_ZOOM_DIST_NEAR  150.0f  // distance at which zoom = MAX
#define CAMERA_ZOOM_DIST_FAR   360.0f  // distance at which zoom = MIN
#define CAMERA_LERP_SPEED 7.0f         // higher = snappier follow

void camera_shake_trigger(camera_t *cam, float strength, float duration);
void camera_update(camera_t *cam, struct pysics_t *p1, struct pysics_t *p2, vec2i_t map_size, float delta_time);
SDL_Rect camera_get_view_rect(const camera_t *cam, vec2i_t map_size);
SDL_Point world_to_screen_point(SDL_Rect view, SDL_Point world);
SDL_Rect world_to_screen_rect(SDL_Rect view, SDL_Rect world);

#ifdef CAMERA_IMPLEMENTATION

#include <math.h>
#include <SDL2/SDL.h>
#include "renderer.h"
#include "fajter.h"

void camera_update(camera_t *cam, pysics_t *p1, pysics_t *p2, vec2i_t map_size, float delta_time)
{
    // ---- target center: midpoint between fighters ----
    vec2f_t target_pos = vec2f_mul(
        vec2f_add(p1->position, p2->position), 
        vec2f(0.5f, 0.5f)
    );

    // ---- target zoom: further apart = zoom out ----
    float dist = fabsf(p1->position.x - p2->position.x);
    float t = (dist - CAMERA_ZOOM_DIST_NEAR) / 
              (CAMERA_ZOOM_DIST_FAR - CAMERA_ZOOM_DIST_NEAR);
    t = (t < 0.0f) ? 0.0f : (t > 1.0f) ? 1.0f : t; // clamp 0-1
    float target_zoom = CAMERA_MAX_ZOOM + t * (CAMERA_MIN_ZOOM - CAMERA_MAX_ZOOM);

    // ---- smooth toward target (avoid snapping every tick) ----
    float lerp_amt = 1.0f - expf(-CAMERA_LERP_SPEED * delta_time);
    cam->position.x += (target_pos.x - cam->position.x) * lerp_amt;
    cam->position.y += (target_pos.y - cam->position.y) * lerp_amt;
    cam->zoom       += (target_zoom - cam->zoom) * lerp_amt;

    // ---- clamp so camera view never shows past the map edge ----
    float half_view_w = (SCREEN_WIDTH  * 0.5f) / cam->zoom;
    float half_view_h = (SCREEN_HEIGHT * 0.5f) / cam->zoom;

    cam->position.x = SDL_clamp(cam->position.x, half_view_w, (float)map_size.x - half_view_w);
    cam->position.y = SDL_clamp(cam->position.y, half_view_h, (float)map_size.y - half_view_h);

    // ---- shake decay (see below) ----
    if (cam->shake_time > 0.0f)
    {
        cam->shake_time -= delta_time;
        float falloff = cam->shake_time > 0.0f ? cam->shake_time : 0.0f;
        cam->shake_offset.x = ((float)rand()/(float)RAND_MAX * 2.0f - 1.0f) * cam->shake_strength * falloff;
        cam->shake_offset.y = ((float)rand()/(float)RAND_MAX * 2.0f - 1.0f) * cam->shake_strength * falloff;
    }
    else
        cam->shake_offset = vec2f(0.0f, 0.0f);

    cam->view = camera_get_view_rect(cam, map_size);
}

// camera_t from before, plus map_size stored wherever match owns the stage size
SDL_Rect camera_get_view_rect(const camera_t *cam, vec2i_t map_size)
{
    // Bigger view_w/view_h = sampling more of the texture = looks zoomed out
    int32_t view_w = (int32_t)((float)SCREEN_WIDTH  / cam->zoom);
    int32_t view_h = (int32_t)((float)SCREEN_HEIGHT / cam->zoom);

    SDL_Rect view;
    view.w = view_w;
    view.h = view_h;
    view.x = (int32_t)((cam->position.x + cam->shake_offset.x) - (float)view_w * 0.5f);
    view.y = (int32_t)((cam->position.y + cam->shake_offset.y) - (float)view_h * 0.5f);

    // Never sample outside the stage texture
    view.x = SDL_clamp(view.x, 0, map_size.x - view_w);
    view.y = SDL_clamp(view.y, 0, map_size.y - view_h);

    return view;
}

void camera_shake_trigger(camera_t *cam, float strength, float duration)
{
    cam->shake_time = duration;
    cam->shake_strength = strength;
}

SDL_Rect world_to_screen_rect(SDL_Rect view, SDL_Rect world)
{
    float scale = (float)SCREEN_WIDTH / (float)view.w;

    SDL_Rect screen;
    screen.x = (int32_t)((float)(world.x - view.x) * scale);
    screen.y = (int32_t)((float)(world.y - view.y) * scale);
    screen.w = (int32_t)((float)world.w * scale);
    screen.h = (int32_t)((float)world.h * scale);
    return screen;
}

SDL_Point world_to_screen_point(SDL_Rect view, SDL_Point world)
{
    float scale = (float)SCREEN_WIDTH / (float)view.w;
    return (SDL_Point){
        (int32_t)((float)(world.x - view.x) * scale),
        (int32_t)((float)(world.y - view.y) * scale)
    };
}

#endif // CAMERA_IMPLEMENTATION

#endif // _CAMERA_H
