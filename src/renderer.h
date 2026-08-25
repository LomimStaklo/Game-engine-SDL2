#ifndef _RENDERER_H
#define _RENDERER_H

// Header only file!
// For implementation you will need to define:
// #define RENDERER_IMPLEMENTATION

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include "macros.h"

// =============
//  DECLARATION
// =============

// ---- SCREEN SIZE ------
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 360

#define MAX_RENDERER_CMDS 256
#define MAX_RENDERER_TEXTURES 256

#define COLOR_RED   (SDL_Color){255,0,0,255}
#define COLOR_GREEN (SDL_Color){0,255,0,255}
#define COLOR_BLUE  (SDL_Color){0,0,255,255}
#define COLOR_WHITE (SDL_Color){255,255,255,255}
#define COLOR_BLACK (SDL_Color){0,0,0,255}

typedef enum render_layer_t 
{
    LAYER_BACKGROUND = 0,
    LAYER_ENTITY,
    LAYER_UI1,
    LAYER_UI2,
    LAYER_COUNT,
} render_layer_t;

/**
 * Index into loaded renderers textures
 */
typedef int32_t texture_handle_t;
#define INVALID_TEXTURE_HANDLE (-1)

typedef struct frame_t
{
    // Frame tile rect from the atlas
    SDL_Rect src;
    vec2i_t offset;
    
    int32_t ticks; // Amount of ticks frame will last (1 tick ~0.016 sec)

    // Collision
    uint8_t  count_hitboxs, count_hurtboxs;
    SDL_Rect hitboxs[1],    hurtboxs[1];
} frame_t;

typedef struct animation_def_t
{
    frame_t frames[10];
    int32_t frame_count;
    int32_t total_ticks;
    bool loop;
} animation_def_t;  

typedef struct animation_t
{
    const animation_def_t *animations; // Pointer to array of anims
    int32_t id;     // Index in animations array of current animation 
    int32_t texture;
    int32_t duration;
    uint16_t timer;  // current tick animation is on
    uint16_t frame; // current frame of animation is on
} animation_t;

typedef struct animated_object_t
{
    animation_t anim;
    vec2f_t position;
    bool facing_right;
} animated_object_t;

typedef enum renderer_command_id_t 
{
    REND_CMD_TEXTURE = 0,
    REND_CMD_TEXTURE_MOD,
    REND_CMD_RECT,
    REND_CMD_LINE,
    REND_CMD_TEXT,
} renderer_command_id_t;

// Tag union for all rendering comands
typedef union renderer_command_t 
{
    uint32_t type;
    struct 
    {
        uint32_t type;
        texture_handle_t handle;  // Texture to be rendered
        SDL_Rect src;
        SDL_Rect dst;
        double rotation;
        SDL_RendererFlip flip;
    } texture;
    struct 
    {
        uint32_t type;
        texture_handle_t handle;  // Texture to be rendered
        SDL_Rect src;
        SDL_Rect dst;
        double rotation;
        SDL_RendererFlip flip;
        SDL_Color col;
    } texture_mod;
    struct 
    {
        uint32_t type;
        SDL_Rect dst;
        SDL_Color col;
        bool is_filled;
    } rect;
    struct 
    {
        uint32_t type;
        SDL_Point p1, p2;
        SDL_Color col;
    } line;
    struct 
    {
        uint32_t type;
        const char *str; 
        int32_t x, y; 
        int32_t chr_w, chr_h; 
        SDL_Color col;
    } text;
} renderer_command_t;

typedef struct renderer_t
{
    renderer_command_t commands[LAYER_COUNT][MAX_RENDERER_CMDS];
    SDL_Texture *textures[MAX_RENDERER_TEXTURES]; // Texture buffers (all textures)

    uint32_t command_count[LAYER_COUNT];
    uint32_t texture_count;
    
    SDL_Renderer *sdl_renderer;
    SDL_Window *sdl_window;
    SDL_Texture *sdl_screen; 

    texture_handle_t font_texture; // Special member 
} renderer_t;

bool init_renderer(renderer_t *renderer);
void destroy_renderer(renderer_t *renderer); // Destroys the renderer with all textures

void animation_init(animation_t *anim, texture_handle_t texture, animation_def_t *defs);
void animation_update(animation_t *anim);
void animation_change(animation_t *anim, int32_t next_anim, bool restart);
int32_t animation_def_total_ticks(const animation_def_t *anim);
const frame_t *animation_get_frame(const animation_t *anim);

texture_handle_t renderer_load_texture(renderer_t *renderer, const char *filename);
texture_handle_t renderer_load_texture_from_mem(renderer_t *renderer, const uint8_t *data, size_t size);
bool renderer_unload_texture(renderer_t *renderer, texture_handle_t tex_handle);
void renderer_texture_size(renderer_t *renderer, texture_handle_t handle, int32_t *w, int32_t *h);

texture_handle_t renderer_load_surface(renderer_t *renderer, SDL_Surface *surf, bool srcfree);
SDL_Texture *renderer_handle_to_texture(renderer_t *renderer, texture_handle_t handle);
void renderer_start_drawing(renderer_t *renderer);
void renderer_present(renderer_t *renderer);

void renderer_draw_texture(
    renderer_t *renderer,
    render_layer_t layer,
    texture_handle_t handle,
    const SDL_Rect *src,
    const SDL_Rect *dst,
    double rotation,
    SDL_RendererFlip flip
);

void renderer_draw_texture_mod(
    renderer_t *renderer,
    render_layer_t layer,
    texture_handle_t handle,
    const SDL_Rect *src,
    const SDL_Rect *dst,
    double rotation,
    SDL_RendererFlip flip,
    SDL_Color col
);

void renderer_draw_rect(
    renderer_t *renderer,
    render_layer_t layer,
    const SDL_Rect *dst,
    SDL_Color color,
    bool do_fill
);

void renderer_draw_line(
    renderer_t *renderer,
    render_layer_t layer,
    SDL_Point point1,
    SDL_Point point2,
    SDL_Color color
);

void renderer_draw_text(
    renderer_t *renderer,
    render_layer_t layer,
    const char *text,
    int32_t x, 
    int32_t y, 
    int32_t chr_w, 
    int32_t chr_h, 
    SDL_Color color
);

void renderer_start_effect(
    renderer_t *renderer,
    render_layer_t layer,
    texture_handle_t handle,  // Texture to be rendered
    const struct animation_def_t *anim,
    int32_t lifetime,
    vec2f_t pos,
    bool facing_right
);

struct fighter_t;
struct projectile_t;
struct pysics_t;

void renderer_draw_animation(renderer_t *renderer, render_layer_t layer, const animated_object_t *anim_obj, SDL_Rect view);
void renderer_draw_fighter(renderer_t *renderer, struct fighter_t *fighter, SDL_Rect view, int32_t floor);
void renderer_draw_projectile(renderer_t *renderer, struct projectile_t *proj, SDL_Rect view, int32_t floor);

// ================
//  IMPLEMENTATION
// ================

#ifdef RENDERER_IMPLEMENTATION

#include <SDL2/SDL_image.h>
#include <string.h>
#include "macros.h"
#include "fajter.h"

#define WIN_FLAGS (SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE /*| SDL_WINDOW_FULLSCREEN_DESKTOP*/)
#define RENDERER_FLAGS (SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)

bool init_renderer(renderer_t *renderer)
{
    renderer->sdl_window = SDL_CreateWindow
    (
        "Street Kebab Fajter",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        WIN_FLAGS
    );
    if (!renderer->sdl_window) { 
        return false;
    } 

    renderer->sdl_renderer = SDL_CreateRenderer(renderer->sdl_window, -1, RENDERER_FLAGS);
    if (!renderer->sdl_renderer) 
        return false;
    
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_SetRenderDrawBlendMode(renderer->sdl_renderer, SDL_BLENDMODE_BLEND);

    renderer->sdl_screen = SDL_CreateTexture(
        renderer->sdl_renderer, 
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        SCREEN_WIDTH, 
        SCREEN_HEIGHT
    );
    
    SDL_SetTextureBlendMode(renderer->sdl_screen, SDL_BLENDMODE_BLEND);
    
    if (!asset_load_all(renderer))
        return false;
    
    renderer->font_texture = asset_get_texture(ASSET_UI_FONT);
    return true;
}

void destroy_renderer(renderer_t *renderer)
{
    for_range_i(renderer->texture_count)
    {
        if (renderer->textures[i]) 
        {
            SDL_DestroyTexture(renderer->textures[i]);
            renderer->textures[i] = NULL;
        }
    }

    SDL_DestroyTexture(renderer->sdl_screen);
    SDL_DestroyRenderer(renderer->sdl_renderer);
    renderer->font_texture = INVALID_TEXTURE_HANDLE;
    renderer->sdl_screen = NULL;
    renderer->sdl_renderer = NULL;
    renderer->texture_count = 0;

    SDL_DestroyWindow(renderer->sdl_window);
    renderer->sdl_window = NULL;
}

SDL_Texture *renderer_handle_to_texture(renderer_t *renderer, texture_handle_t handle)
{
    if (!is_in_range(0, (int32_t)renderer->texture_count, handle))
        return NULL;

    return renderer->textures[handle];
}

void renderer_texture_size(renderer_t *renderer, texture_handle_t handle, int32_t *w, int32_t *h)
{
    SDL_Texture *tex = renderer_handle_to_texture(renderer, handle);
    if (!tex) return;

    SDL_QueryTexture(tex, NULL, NULL, w, h);
}

texture_handle_t renderer_load_texture(renderer_t *renderer, const char *filename)
{
    if (renderer->texture_count >= MAX_RENDERER_TEXTURES) 
        return INVALID_TEXTURE_HANDLE;
    
    SDL_Surface *surf = IMG_Load(filename);
    if (!surf) 
        return INVALID_TEXTURE_HANDLE;
        
    SDL_Texture *rend_tex = SDL_CreateTextureFromSurface(renderer->sdl_renderer, surf);
    SDL_FreeSurface(surf);
        
    if (!rend_tex) 
        return INVALID_TEXTURE_HANDLE;
    
    texture_handle_t handle = INVALID_TEXTURE_HANDLE;
    renderer->texture_count++;

    for_range_i(renderer->texture_count)
        if (renderer->textures[i] == NULL)
        {   
            handle = (texture_handle_t)i;
            renderer->textures[i] = rend_tex;
            break;
        }

    return handle;
}

texture_handle_t renderer_load_texture_from_mem(renderer_t *renderer, const uint8_t *data, size_t size)
{
    if (renderer->texture_count >= MAX_RENDERER_TEXTURES) 
        return INVALID_TEXTURE_HANDLE;
    
    SDL_RWops *raw_bytes = SDL_RWFromConstMem((const void *)data, (int32_t)size);
    SDL_Texture *texture = IMG_LoadTexture_RW(renderer->sdl_renderer, raw_bytes, 1);
    //game_log( "ERROR", "SDL renderer: %s: s:%zu", SDL_GetError(), size);
    
    if (!texture)
        return INVALID_TEXTURE_HANDLE;
    
    texture_handle_t handle = INVALID_TEXTURE_HANDLE;
    renderer->texture_count++;

    for_range_i(renderer->texture_count) 
    {
        if (renderer->textures[i] == NULL)
        {   
            handle = (texture_handle_t)i;
            renderer->textures[i] = texture;
            break;
        }
    }
    return handle;
}

texture_handle_t renderer_load_surface(renderer_t *renderer, SDL_Surface *surf, bool srcfree)
{
    if (renderer->texture_count >= MAX_RENDERER_TEXTURES) 
        return INVALID_TEXTURE_HANDLE;
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer->sdl_renderer, surf);
    if (srcfree) SDL_FreeSurface(surf);
    if (!texture)
        return INVALID_TEXTURE_HANDLE;
    
    texture_handle_t handle = INVALID_TEXTURE_HANDLE;
    renderer->texture_count++;

    for_range_i(renderer->texture_count) 
    {
        if (renderer->textures[i] == NULL)
        {   
            handle = (texture_handle_t)i;
            renderer->textures[i] = texture;
            break;
        }
    }
    return handle;
}

bool renderer_unload_texture(renderer_t *renderer, texture_handle_t tex_handle)
{
    if (!is_in_range(0, (int32_t)renderer->texture_count, tex_handle))
        return false;
    
    if (renderer->textures[tex_handle])
        SDL_DestroyTexture(renderer->textures[tex_handle]);
    renderer->textures[tex_handle] = NULL;
    return true;
}

int32_t animation_def_total_ticks(const animation_def_t *anim)
{
    int32_t ticks = 0;
    for_range_i((unsigned)anim->frame_count)
        ticks += anim->frames[i].ticks;
    return ticks;
}

void animation_init(animation_t *anim, texture_handle_t texture, animation_def_t *defs)
{
    *anim = (animation_t) {
        .animations = defs,
        .texture = texture,
        .duration = animation_def_total_ticks(&defs[0])
    };
}

void animation_update(animation_t *anim)
{
    anim->timer++;
    
    const animation_def_t *curr = &anim->animations[anim->id];
    int32_t frame_timestamp = 0;
    
    for_range_i((unsigned)anim->frame + 1)
        frame_timestamp += curr->frames[i].ticks;

    if (anim->timer >= frame_timestamp)
    {
        anim->frame++;
        
        if (anim->frame >= curr->frame_count)
        {    
            anim->frame =
                (curr->loop) ? 0 : (uint16_t)curr->frame_count - 1;
            anim->timer =
                (curr->loop) ? 0 : (uint16_t)animation_def_total_ticks(curr);
        }
    }
}

const frame_t *animation_get_frame(const animation_t *anim)
{
    return &anim->animations[anim->id].frames[anim->frame];
}

void animation_change(animation_t *anim, int32_t next_anim, bool restart)
{
    if (anim->id != next_anim) 
    {
        anim->id = next_anim;
        anim->timer = TICKS(0);
        
        anim->frame = (restart) ? 0 : (uint16_t)anim->animations[anim->id].frame_count - 1;
        anim->timer = (restart) ? TICKS(0) : (uint16_t)animation_def_total_ticks(&anim->animations[anim->id]) - 1;
    }
}

void renderer_start_drawing(renderer_t *renderer)
{
    for_range_i(LAYER_COUNT)
        renderer->command_count[i] = 0;
    
    SDL_SetRenderTarget(renderer->sdl_renderer, renderer->sdl_screen);
    SDL_RenderClear(renderer->sdl_renderer);
}

void renderer_present(renderer_t *renderer)
{
    for (uint32_t layer = 0; layer < LAYER_COUNT; layer++)
    {
        for_range_i(renderer->command_count[layer])
        {
            renderer_command_t *cmd = &renderer->commands[layer][i];
            
            // ---- RENDERING ---------------------------------------------------------------------------
            switch (cmd->type)
            {
                // ---- TEXTURE -------------------------------------------------------------------------
                case REND_CMD_TEXTURE:
                {
                    SDL_RenderCopyEx(
                        renderer->sdl_renderer,
                        renderer->textures[cmd->texture.handle],
                        &cmd->texture.src,
                        &cmd->texture.dst,
                        cmd->texture.rotation,
                        NULL,
                        cmd->texture.flip
                    );

                    break;
                }
                // ---- TEXTURE MOD ---------------------------------------------------------------------
                case REND_CMD_TEXTURE_MOD:
                {
                    SDL_Color mod_col = cmd->texture_mod.col; 
                    
                    SDL_SetTextureColorMod(
                        renderer->textures[cmd->texture_mod.handle], 
                        mod_col.r, mod_col.g, mod_col.b
                    );
                    SDL_SetTextureAlphaMod(
                        renderer->textures[cmd->texture_mod.handle], 
                        mod_col.a
                    );
                    
                    SDL_RenderCopyEx(
                        renderer->sdl_renderer,
                        renderer->textures[cmd->texture_mod.handle],
                        &cmd->texture_mod.src,
                        &cmd->texture_mod.dst,
                        cmd->texture_mod.rotation,
                        NULL,
                        cmd->texture_mod.flip
                    );
                     
                    SDL_SetTextureColorMod(
                        renderer->textures[cmd->texture_mod.handle], 
                        (uint8_t)(mod_col.r + 255) % 256, 
                        (uint8_t)(mod_col.g + 255) % 256, 
                        (uint8_t)(mod_col.b + 255) % 256
                    );
                    SDL_SetTextureAlphaMod(
                        renderer->textures[cmd->texture_mod.handle], 
                        255
                    );

                    break;
                }
                // ---- RECT ----------------------------------------------------------------------------
                case REND_CMD_RECT:
                {
                    SDL_SetRenderDrawColor(renderer->sdl_renderer, 
                        cmd->rect.col.r, 
                        cmd->rect.col.g, 
                        cmd->rect.col.b, 
                        cmd->rect.col.a
                    );

                    if(cmd->rect.is_filled) 
                        SDL_RenderFillRect(renderer->sdl_renderer, &cmd->rect.dst);
                    else
                        SDL_RenderDrawRect(renderer->sdl_renderer, &cmd->rect.dst);
                    break;
                }
                // ---- LINE ----------------------------------------------------------------------------
                case REND_CMD_LINE:
                {
                    SDL_SetRenderDrawColor(renderer->sdl_renderer, 
                        cmd->line.col.r,
                        cmd->line.col.g,
                        cmd->line.col.b,
                        cmd->line.col.a
                    );

                    SDL_RenderDrawLine(
                        renderer->sdl_renderer,
                        cmd->line.p1.x, 
                        cmd->line.p1.y, 
                        cmd->line.p2.x, 
                        cmd->line.p2.y
                    );
                    break;
                }
                // ---- TEXT ----------------------------------------------------------------------------
                case REND_CMD_TEXT: 
                {
                    SDL_Texture *font = renderer->textures[renderer->font_texture];
                    SDL_SetTextureColorMod(
                        font, 
                        cmd->text.col.r,
                        cmd->text.col.g,
                        cmd->text.col.b
                    );
                    // Constants that work on the specific font baked in the file text.h
                    const int32_t FONT_W = 64, FONT_H = 64, FONT_ATLAS_COLUMS = 16;
                    
                    const uint32_t text_len = (uint32_t)strlen(cmd->text.str);
                    // Where the characters (text rect of a char) will be rendered 
                    SDL_Rect dst = { 
                        cmd->text.x, cmd->text.y, 
                        cmd->text.chr_w, cmd->text.chr_h
                    };

                    for_range_j(text_len)
                    {
                        const char chr = cmd->text.str[j];
                        if (chr < 32 || chr >= 127)
                        {
                            if (chr == '\n')
                            {
                                dst.x = cmd->text.x;
                                dst.y += cmd->text.chr_h;
                            }
                            continue;
                        }
                        // Renders the text to screen rect by rect, char by char
                        SDL_Rect src = tile_from_atlas((chr - 32), FONT_W, FONT_H, FONT_ATLAS_COLUMS);
                        SDL_RenderCopy(renderer->sdl_renderer, font, &src, &dst);

                        dst.x += dst.w;
                    }
                    
                    break;
                }
            }
            
        }
    }

    SDL_SetRenderTarget(renderer->sdl_renderer, NULL);
    SDL_SetRenderDrawColor(renderer->sdl_renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer->sdl_renderer);

    SDL_RenderCopy(
        renderer->sdl_renderer,
        renderer->sdl_screen,
        NULL,
        NULL
    );

    SDL_RenderPresent(renderer->sdl_renderer);
}

// ---- TEXTURE -------------------------------------------------------------------------
// if dst or src is NULL it gets rendered to whole screen
void renderer_draw_texture(
    renderer_t *renderer,
    render_layer_t layer,
    texture_handle_t handle,
    const SDL_Rect *src,
    const SDL_Rect *dst,
    double rotation,
    SDL_RendererFlip flip) 
{
    if (renderer->command_count[layer] >= MAX_RENDERER_CMDS)
        return;
    
    assert(is_in_range(0, (int32_t)renderer->texture_count, handle) &&
            "Texture handle out of bounds");

    SDL_Rect src_rect = {0, 0, 0, 0};
    if (src == NULL)
    {
        SDL_QueryTexture(
            renderer->textures[handle], 
            NULL, 
            NULL,
            &src_rect.w,
            &src_rect.h
        );
    } else
        src_rect = *src;

    SDL_Rect dst_rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    if (dst != NULL)  
        dst_rect = *dst;

    renderer->commands[layer][renderer->command_count[layer]] = (renderer_command_t) 
    {
        .texture = 
        {
            .type = REND_CMD_TEXTURE,
            .handle = handle,
            .src = src_rect,
            .dst = dst_rect,
            .rotation = rotation,
            .flip = flip,
        }
    };
    renderer->command_count[layer] += 1;
}

void renderer_draw_texture_mod(
    renderer_t *renderer,
    render_layer_t layer,
    texture_handle_t handle,
    const SDL_Rect *src,
    const SDL_Rect *dst,
    double rotation,
    SDL_RendererFlip flip,
    SDL_Color col) 
{
    if (renderer->command_count[layer] >= MAX_RENDERER_CMDS)
        return;
    
    assert(is_in_range(0, (int32_t)renderer->texture_count, handle) &&
            "Texture handle out of bounds");

    SDL_Rect src_rect = {0, 0, 0, 0};
    if (src == NULL)
    {
        SDL_QueryTexture(
            renderer->textures[handle], 
            NULL, 
            NULL,
            &src_rect.w,
            &src_rect.h
        );
    } 
    else
        src_rect = *src;

    SDL_Rect dst_rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    if (dst != NULL)  
        dst_rect = *dst;

    renderer->commands[layer][renderer->command_count[layer]] = (renderer_command_t) 
    {
        .texture_mod = 
        {
            .type = REND_CMD_TEXTURE_MOD,
            .handle = handle,
            .src = src_rect,
            .dst = dst_rect,
            .rotation = rotation,
            .flip = flip,
            .col = col
        }
    };
    renderer->command_count[layer] += 1;
}


// ---- RECT ----------------------------------------------------------------------------
// if dst is NULL it gets rendered to whole screen
void renderer_draw_rect(
    renderer_t *renderer,
    render_layer_t layer,
    const SDL_Rect *dst,
    SDL_Color color,
    bool do_fill)
{
    if (renderer->command_count[layer] >= MAX_RENDERER_CMDS)
        return;
        
    SDL_Rect dst_rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    if (dst != NULL)  
        dst_rect = *dst;
    
    renderer->commands[layer][renderer->command_count[layer]] = (renderer_command_t) 
    {
        .rect =
        {
            .type = REND_CMD_RECT,
            .dst = dst_rect,
            .col = color,
            .is_filled = do_fill
        }
    };
    renderer->command_count[layer] += 1;
}

// ---- LINE ----------------------------------------------------------------------------
void renderer_draw_line(
    renderer_t *renderer,
    render_layer_t layer,
    SDL_Point point1,
    SDL_Point point2,
    SDL_Color color)
{
    if (renderer->command_count[layer] >= MAX_RENDERER_CMDS)
        return;
    
    renderer->commands[layer][renderer->command_count[layer]] = (renderer_command_t) 
    {
        .line = 
        {
            .type = REND_CMD_LINE,
            .p1  = point1,
            .p2  = point2,
            .col = color
        }
    };
    renderer->command_count[layer] += 1;
}

// ---- TEXT ----------------------------------------------------------------------------
void renderer_draw_text(
    renderer_t *renderer,
    render_layer_t layer,
    const char *text,
    int32_t x, 
    int32_t y, 
    int32_t chr_w, 
    int32_t chr_h, 
    SDL_Color color)
{
    if (renderer->command_count[layer] >= MAX_RENDERER_CMDS)
        return;
    
    renderer->commands[layer][renderer->command_count[layer]] = (renderer_command_t) 
    {
        .text = 
        {
            .type  = REND_CMD_TEXT,
            .str   = text,
            .x     = x,
            .y     = y,
            .chr_w = chr_w,
            .chr_h = chr_h,
            .col   = color
        }
    };
    renderer->command_count[layer] += 1;
}

void renderer_draw_animation(renderer_t *renderer, render_layer_t layer, const animated_object_t *anim_obj, SDL_Rect view)
{
    const frame_t *frame = animation_get_frame(&anim_obj->anim);

    pysics_t pysics = {
        .position = anim_obj->position, 
        .facing_right = anim_obj->facing_right
    };

    SDL_Rect world = frame_rect_facing_position(frame, &pysics);
    SDL_Rect dst   = world_to_screen_rect(view, world);
    
    renderer_draw_texture(
        renderer,
        layer,
        anim_obj->anim.texture,
        &frame->src,
        &dst, 
        0.0, 
        pysics.facing_right ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE
    );
}

#define DBG_BOXES 0
// ---- FIGHTER -------------------------------------------------------------------------
void renderer_draw_fighter(renderer_t *renderer, fighter_t *fighter, SDL_Rect view, int32_t floor)
{   
    const frame_t *frame = animation_get_frame(&fighter->animation);
    
    // Direction corection
    SDL_Rect world = frame_rect_facing_position(frame, &fighter->pysics);
    SDL_Rect dst   = world_to_screen_rect(view, world);

    renderer_draw_texture(
        renderer, 
        LAYER_ENTITY,
        fighter->animation.texture, 
        &frame->src,
        &dst, 
        0.0,
        fighter->pysics.facing_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL
    );
    
    // Shadow rendering
    SDL_Rect shadow = world;
    shadow.y = (int32_t)floor - 8;
    shadow.h -= SDL_clamp((floor - (int32_t)fighter->pysics.position.y), 0, 40); 
    shadow = world_to_screen_rect(view, shadow);

    renderer_draw_texture_mod(
        renderer, 
        LAYER_ENTITY,
        fighter->animation.texture, 
        &frame->src,
        &shadow, 
        180.0, 
        fighter->pysics.facing_right ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE,
        (SDL_Color){0, 0, 0, 128}
    );
    
    // Debug collision rects 
    if (DBG_BOXES) 
    {
        for_range_j(frame->count_hurtboxs)
        {
            SDL_Rect hurt = fighter_to_world_rect(fighter, frame->hurtboxs[j]);
            hurt = world_to_screen_rect(view, hurt);
            renderer_draw_rect(renderer, LAYER_UI1, &hurt, COLOR_GREEN, false);
        }
        for_range_i(frame->count_hitboxs)
        {
            SDL_Rect hit = fighter_to_world_rect(fighter, frame->hitboxs[i]);
            hit = world_to_screen_rect(view, hit);
            renderer_draw_rect(renderer, LAYER_UI1, &hit, COLOR_RED, false);
        }
    }
}

void renderer_draw_projectile(renderer_t *renderer, projectile_t *proj, SDL_Rect view, int32_t floor)
{
    const frame_t *frame = animation_get_frame(&proj->anim);

    SDL_Rect world = frame_rect_facing_position(frame, &proj->pysics);
    SDL_Rect dst   = world_to_screen_rect(view, world);

    renderer_draw_texture(
        renderer, 
        LAYER_ENTITY,
        proj->anim.texture, 
        &frame->src,
        &dst, 
        0.0,
        proj->pysics.facing_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL
    );
    
    // Shadow rendering
    SDL_Rect shadow = world;
    shadow.y = (int32_t)floor - 8;
    shadow.h -= SDL_clamp((floor - (int32_t)proj->pysics.position.y), 0, 40); 
    shadow = world_to_screen_rect(view, shadow);

    renderer_draw_texture_mod(
        renderer,
        LAYER_ENTITY,
        proj->anim.texture, 
        &frame->src,
        &shadow, 
        180.0, 
        proj->pysics.facing_right ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE,
        (SDL_Color){0, 0, 0, 128}
    );
    
    // Debug collision rects 
    if (DBG_BOXES) 
    {
        for_range_j(frame->count_hurtboxs)
        {
            SDL_Rect hurt = to_world_rect(&proj->pysics, frame, frame->hurtboxs[j]);
            hurt = world_to_screen_rect(view, hurt);
            renderer_draw_rect(renderer, LAYER_UI1, &hurt, COLOR_GREEN, false);
        }
        for_range_i(frame->count_hitboxs)
        {
            SDL_Rect hit = to_world_rect(&proj->pysics, frame, frame->hitboxs[i]);
            hit = world_to_screen_rect(view, hit);
            renderer_draw_rect(renderer, LAYER_UI1, &hit, COLOR_RED, false);
        }
    }
}


#endif /* RENDERER_IMPLEMENTATION */

#endif /* !_RENDERER_H */
