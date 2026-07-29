#ifndef _CHARACTERS_H
#define _CHARACTERS_H

// Header only file!
// For implementation you will need to define:
// #define CHARACTERS_IMPLEMENTATION

#include "fajter.h"

// =============
//  DECLARATION
// =============

// X macro for all playable character 
#define CHARACTERS_XLIST \
X(BOKE) 
// ----------------------------------

typedef enum character_name_t 
{
#define X(name) CHARACTER_ ## name,
    CHARACTERS_XLIST
    CHARACTER_COUNT
#undef X
} character_name_t;

/**
 * Get the character by name with texture 
 * \param renderer renderer
 * \param name the character name
 * \param palette color palette of character, default is 0
 */
void character_constructor(fighter_t *fg, renderer_t *renderer, character_name_t name, float hue);

// ================
//  IMPLEMENTATION
// ================

#ifdef CHARACTERS_IMPLEMENTATION
#include <string.h>
#include <math.h>
#include "macros.h"

#define ANIM(lp, total, ...) \
    { .loop = (lp), \
      .total_ticks = (total), \
      .frames = {__VA_ARGS__}, \
      .frame_count = (sizeof((frame_t[]){__VA_ARGS__}) / sizeof(frame_t)) }

#define FRAME_HURT(src_r, tick, offx, offy, hx, hy, hw, hh) \
    { .src = src_r, .ticks = (tick), .offset.x = (offx), .offset.y = (offy), \
      .hurtboxs[0] = {(hx), (hy), (hw), (hh)}, .count_hurtboxs = 1 }

#define FRAME_IMMUNE(src_r, tick, offx, offy) \
    { .src = src_r, .ticks = (tick), .offset.x = (offx), .offset.y = (offy) }

#define FRAME_HIT(src_r, tick, offx, offy, hurtx, hurty, hurtw, hurth, hitx, hity, hitw, hith) \
    { .src = src_r, .ticks = (tick), .offset.x = (offx), .offset.y = (offy), \
      .hurtboxs[0] = {(hurtx), (hurty), (hurtw), (hurth)}, .count_hurtboxs = 1, \
      .hitboxs[0]  = {(hitx),  (hity),  (hitw),  (hith)},  .count_hitboxs  = 1 }
 
/**
 * Starting point of 48 by 96 frames in a atlas
 * The atlas is divided in different regions sorted by tile size
 * 48x96 start at origin of (0, 0)
 * \returns Rectangle (SDL_Rect) of the tile in a atlas 
 */
#define TILE_48x96(idx) tile_from_atlas_xy(idx, 0, 0,   48, 96, 21)
#define TILE_64x96(idx) tile_from_atlas_xy(idx, 0, 96,  64, 96, 16)
#define TILE_96x96(idx) tile_from_atlas_xy(idx, 0, 288, 96, 96, 10)

// Magic number where the palette starts in atlas 
#define CHARACTER_X_COORDEINATE_FOR_PALETTE 1008

static const fighter_def_t fajter_characters_defs[CHARACTER_COUNT];

typedef struct color_hsv_t
{
    float h; // Hue: 0.0 to 360.0 degrees
    float s; // Saturation: 0.0 to 1.0
    float v; // Value/Brightness: 0.0 to 1.0
} color_hsv_t;

static color_hsv_t RGB_to_HSV(SDL_Color col) 
{
    color_hsv_t hsv;
    
    // Normalize RGB values to 0.0 - 1.0 range
    float r = col.r / 255.0f;
    float g = col.g / 255.0f;
    float b = col.b / 255.0f;

    // Find the minimum and maximum values among R, G, B
    float max = (r > g) ? ((r > b) ? r : b) : ((g > b) ? g : b);
    float min = (r < g) ? ((r < b) ? r : b) : ((g < b) ? g : b);
    float delta = max - min;

    // 1. Calculate Value (Brightness)
    hsv.v = max;

    // 2. Calculate Saturation
    if (max > 0.0f) {
        hsv.s = delta / max;
    } else {
        // R = G = B = 0 (Black)
        hsv.s = 0.0f;
        hsv.h = 0.0f; 
        return hsv;
    }

    // 3. Calculate Hue
    if (delta == 0.0f) {
        hsv.h = 0.0f; // Achromatic case (Gray, White)
    } else {
        if (is_in_range(r + 0.1f, r - 0.1f, max)) 
        {
            hsv.h = 60.0f * (g - b) / delta;
        } else if (is_in_range(g + 0.1f, g - 0.1f, max)) 
        {
            hsv.h = 60.0f * (b - r) / delta + 120.0f;
        } else {
            hsv.h = 60.0f * (r - g) / delta + 240.0f;
        }

        // Keep hue positive within 0-360 range
        if (hsv.h < 0.0f) hsv.h += 360.0f;
    }

    return hsv;
}

static SDL_Color HSV_to_RGB(color_hsv_t hsv) 
{
    float c = hsv.v * hsv.s;
    float h_prime = (float)fmod((double)hsv.h / 60.0, 6.0);
    float x = c * (float)(1.0 - fabs(fmod((double)h_prime, 2.0) - 1.0));
    float m = hsv.v - c;

    float r1 = 0, g1 = 0, b1 = 0;

    if (h_prime >= 0 && h_prime < 1) { r1 = c; g1 = x; b1 = 0; }
    else if (h_prime >= 1 && h_prime < 2) { r1 = x; g1 = c; b1 = 0; }
    else if (h_prime >= 2 && h_prime < 3) { r1 = 0; g1 = c; b1 = x; }
    else if (h_prime >= 3 && h_prime < 4) { r1 = 0; g1 = x; b1 = c; }
    else if (h_prime >= 4 && h_prime < 5) { r1 = x; g1 = 0; b1 = c; }
    else if (h_prime >= 5 && h_prime < 6) { r1 = c; g1 = 0; b1 = x; }

    SDL_Color rgb;
    rgb.r = (Uint8)((r1 + m) * 255);
    rgb.g = (Uint8)((g1 + m) * 255);
    rgb.b = (Uint8)((b1 + m) * 255);
    rgb.a = 255; // Fully opaque

    return rgb;
}

/**
 * This is a unreadable function that changes the palette of a fighter by looking at a 
 * specific place to find a palette to switch
 * \param surf surface of atlas
 * \param index_x x coordinate where the palette of 10 pixels is
 * \param hue the color hur of new palate 
 */
static void character_switch_palette(SDL_Surface *surf, uint32_t index_x, float hue)
{
    if (!surf) return;

    // Pitch = SCREEN_WIDTH * bytes-per-pixel
    uint32_t pixel_pitch = (uint32_t)surf->pitch / 4; 
    uint32_t *pixels = (uint32_t *)surf->pixels;
    
    // New and default palette
    uint32_t def_pale[10] = {0};
    uint32_t new_pale[10] = {0}; 
    
    for_range_i(lenghtof(def_pale))
    {
        // index = (y * pixel_pitch) + x
        def_pale[i] = pixels[(0 * pixel_pitch) + (index_x + i)]; 
        
        SDL_Color rgb;
        SDL_GetRGBA(def_pale[i], surf->format, &rgb.r, &rgb.g, &rgb.b, &rgb.a);
        
        // Color is converted to hsv and saves the alpha 
        color_hsv_t hsv = RGB_to_HSV(rgb);
        uint8_t alpha = rgb.a;
        
        hsv.h = hue;
        rgb = HSV_to_RGB(hsv); 

        new_pale[i] = SDL_MapRGBA(surf->format, rgb.r, rgb.g, rgb.b, alpha); 
    }

    // Here it applys the pallate to the surface 
    for (uint32_t y = 0; y < (uint32_t)surf->h; ++y) {
        for (uint32_t x = 0; x < (uint32_t)surf->w; ++x) {
        
            // Find precise pixel index matching the pitch offset
            uint32_t index = (y * pixel_pitch) + x;
            uint32_t current_pixel = pixels[index];

            // Current pixel color
            uint8_t r, g, b, a;
            SDL_GetRGBA(current_pixel, surf->format, &r, &g, &b, &a);

            for_range_i(lenghtof(def_pale))
            {
                // Pallet RGBA
                uint8_t r_pall, g_pall, b_pall, a_pall;
                SDL_GetRGBA(def_pale[i], surf->format, &r_pall, &g_pall, &b_pall, &a_pall);

                if (r == r_pall && g == g_pall && b == b_pall)
                {
                    uint8_t r_npall, g_npall, b_npall, a_npall;
                    SDL_GetRGBA(new_pale[i], surf->format, &r_npall, &g_npall, &b_npall, &a_npall);

                    pixels[index] = SDL_MapRGBA(surf->format, r_npall, g_npall, b_npall, a);
                }
            }
        }
    }

    // IMG_SavePNG(surf, "atlas_foo.png");
    return;
}

void character_constructor(fighter_t *fg, renderer_t *renderer, character_name_t name, float hue)
{
    if (!is_in_range(0, CHARACTER_COUNT, (int32_t)name))
        name = CHARACTER_BOKE;

    fg->def = &fajter_characters_defs[name];

    if (hue > 0.0f)
    {
        SDL_Surface *surf = asset_load_as_surface(fg->def->default_asset);
        character_switch_palette(surf, CHARACTER_X_COORDEINATE_FOR_PALETTE, hue);
        
        fg->animation.texture = renderer_load_surface(renderer, surf, true);
    }
    else 
        fg->animation.texture = asset_get_texture(fg->def->default_asset);

    // Point the runtime fighter at its definition
    fg->hp             = fg->def->base_hp;
    fg->ragebait_meter = 100;
    fg->hit_landed_at  = -1;

    // Gets the pointer to array of animation that are indexex by state id
    fg->animation.animations = &fajter_characters_defs[name].animations[0];
}

static const fighter_def_t fajter_characters_defs[CHARACTER_COUNT] =
{
    [CHARACTER_BOKE] =
    {
        .name = "boke",
        .default_asset = ASSET_ATLAS_BOKE,
        .base_hp = 220,
        .walk_speed = 180.0f,
        .jump_force = 350.0f,
            
        // ANIM(frame_duration, loop, startup_frames, active_frames, frame_data(src, x, y, w, h))
        // Animations with size 48x96
        .animations[STATE_IDLE] = ANIM(true, TICKS(3 * 15),
            FRAME_HURT(TILE_48x96(0), TICKS(15), 24, 88,  12, 12, 24, 72),
            FRAME_HURT(TILE_48x96(1), TICKS(15), 24, 88,  12, 12, 24, 72),
            FRAME_HURT(TILE_48x96(2), TICKS(15), 24, 88,  12, 12, 24, 72)
        ),
                
        .animations[STATE_STAND_BLOCK] = ANIM(false, TICKS(15),
            FRAME_HURT(TILE_48x96(3), TICKS(15), 24, 88,  12, 12, 24, 72)
        ),
            
        .animations[STATE_CROUCH] = ANIM(false, TICKS(4 * 5), 
            FRAME_HURT(TILE_48x96(4), TICKS(5), 24, 88,  8, 48, 36, 36),
            FRAME_HURT(TILE_48x96(5), TICKS(5), 24, 88,  8, 48, 36, 36),
            FRAME_HURT(TILE_48x96(6), TICKS(5), 24, 88,  8, 48, 36, 36),
            FRAME_HURT(TILE_48x96(7), TICKS(5), 24, 88,  8, 48, 36, 36)
        ),

        .animations[STATE_CROUCH_BLOCK] = ANIM(false, TICKS(15),
            FRAME_HURT(TILE_48x96(8), TICKS(15), 24, 88,  8, 48, 36, 36)
        ),

        .animations[STATE_STAND_HITSTUN] = ANIM(false, TICKS(15),
            FRAME_HURT(TILE_48x96(9), TICKS(15), 24, 88,  12, 12, 24, 72)
        ),

        .animations[STATE_CROUCH_HITSTUN] = ANIM(false, TICKS(15), 
            FRAME_HURT(TILE_48x96(10), TICKS(15), 24, 88,  8, 48, 36, 36)
        ),

        .animations[STATE_WALK_BACKWARD] = ANIM(true, TICKS(3 * 15),
            FRAME_HURT(TILE_48x96(11), TICKS(15), 32, 88,  8, 16, 24, 68),
            FRAME_HURT(TILE_48x96(12), TICKS(15), 32, 88,  8, 16, 24, 68),
            FRAME_HURT(TILE_48x96(13), TICKS(15), 32, 88,  8, 16, 24, 68)
        ), // 8, 16, 24, 68
            
        .animations[STATE_WALK_FORWARD] = ANIM(true, TICKS(3 * 15),
            FRAME_HURT(TILE_48x96(14), TICKS(15), 24, 88,  20, 16, 24, 68),
            FRAME_HURT(TILE_48x96(15), TICKS(15), 24, 88,  20, 16, 24, 68),
            FRAME_HURT(TILE_48x96(16), TICKS(15), 24, 88,  20, 16, 24, 68),
        ), // 20, 16, 24, 68

        .animations[STATE_AIRBORNE] = ANIM(false, TICKS(15 + 5 + 5),
            FRAME_HURT(TILE_48x96(18), TICKS(15), 24, 88,  8, 40, 36, 44),
            FRAME_HURT(TILE_48x96(17), TICKS(5),  24, 88,  8, 40, 36, 44),
            FRAME_HURT(TILE_48x96(19), TICKS(5),  24, 88,  8, 40, 36, 44),
        ),

        .animations[STATE_AIRBORNE_HITSTUN] = ANIM(false, TICKS(15),
            FRAME_HURT(TILE_48x96(20), TICKS(15), 24, 88,  8, 40, 36, 44)
        ),
            
        // ---- SIZE 64x96 ---------------------------------------------------
        .animations[STATE_STAND_LIGHT] = ANIM(false, TICKS(7 + 3 + 5),
            FRAME_HURT(TILE_64x96(0), TICKS(7), 24, 88,  24, 12, 24, 72),
            FRAME_HIT (TILE_64x96(1), TICKS(3), 16, 88,  16, 12, 24, 72,  32, 24, 32, 8),
            FRAME_HURT(TILE_64x96(1), TICKS(5), 24, 88,  24, 12, 24, 72),
        ),
        .attacks[ATK_ID_STAND_LIGHT] = 
        {   
            .kind   = ATK_KIND_SIMPLE,
            .triger = ATK_TRIGGER_ON_HIT,
            
            .stats.damage = 10,
            .stats.stun_duration = TICKS(9),
            .stats.knockback     = {10.0f, 0.0f},
            .stats.recoil        = {100.0f, 0.0f},
            .stats.flags  = ATK_FLAG_CANCEABLE,
            
            .startup = TICKS(7),
            .active = TICKS(3),
        }, 
        
        .animations[STATE_CROUCH_LIGHT] = ANIM(false, TICKS(7 + 3 + 4), 
            FRAME_HURT(TILE_64x96(2), TICKS(7), 16, 88,  8, 48, 36, 36),
            FRAME_HIT (TILE_64x96(3), TICKS(3), 16, 88,  8, 48, 36, 36,   32, 56, 32, 8),
            FRAME_HURT(TILE_64x96(3), TICKS(4), 16, 88,  8, 48, 36, 36),
        ),
        .attacks[ATK_ID_CROUCH_LIGHT] = 
        {
            .kind   = ATK_KIND_SIMPLE,
            .triger = ATK_TRIGGER_ON_HIT,

            .stats.damage = 10,
            .stats.stun_duration = TICKS(9),
            .stats.knockback     = {50.0f, 0.0f},  
            .stats.recoil        = {100.0f, 0.0f},

            .startup = TICKS(7),
            .active = TICKS(3),
        },

        .animations[STATE_STAND_MEDIUM] = ANIM(false, TICKS(6 + 5 + 7),
            FRAME_HURT(TILE_64x96(4), TICKS(6), 24, 88,  24, 12, 24, 72),
            FRAME_HIT (TILE_64x96(5), TICKS(5), 24, 88,   8, 12, 24, 72,  32, 24, 32, 24),
            FRAME_HURT(TILE_64x96(5), TICKS(6), 24, 88,   8, 12, 24, 72),
        ),
        .attacks[ATK_ID_STAND_MEDIUM] = 
        {
            .kind   = ATK_KIND_SIMPLE,
            .triger = ATK_TRIGGER_ON_HIT,

            .stats.damage = 15, 
            .stats.stun_duration = TICKS(10),
            .stats.knockback = {20.0f, 0.0f}, 
            .stats.recoil    = {150.0f, 0.0f},

            .startup = TICKS(6),
            .active = TICKS(5),
        },

        .animations[STATE_CROUCH_MEDIUM] = ANIM(false, TICKS(6 + 5 + 7),
            FRAME_HURT(TILE_64x96(6), TICKS(6), 24, 88,   8, 48, 36, 36),
            FRAME_HIT (TILE_64x96(7), TICKS(5),  8, 88,   8, 48, 36, 36,  32, 56, 32, 8),
            FRAME_HURT(TILE_64x96(7), TICKS(7),  8, 88,   8, 48, 36, 36),
        ),
        .attacks[ATK_ID_CROUCH_MEDIUM] = 
        {
            .kind   = ATK_KIND_SIMPLE,
            .triger = ATK_TRIGGER_ON_HIT,

            .stats.damage = 15,
            .stats.stun_duration = TICKS(15),
            .stats.knockback = {50.0f, 0.0f}, 
            .stats.recoil    = {200.0f, 0.0f}, 

            .startup = TICKS(6),
            .active = TICKS(5),
        },
            
        .animations[STATE_STAND_HEAVY] = ANIM(false, TICKS(5 * 5),
            FRAME_HURT(TILE_64x96(8),  TICKS(5), 24, 88,  24, 12, 24, 72),
            FRAME_HURT(TILE_64x96(9),  TICKS(5), 24, 88,  24, 12, 24, 72),
            FRAME_HIT (TILE_64x96(10), TICKS(5),  8, 88,   8, 12, 24, 72,  32, 24, 32, 32),
            FRAME_HURT(TILE_64x96(10), TICKS(5),  8, 88,   8, 12, 24, 72),
            FRAME_HURT(TILE_64x96(9),  TICKS(5), 24, 88,  24, 12, 24, 72),
        ),
        .attacks[ATK_ID_STAND_HEAVY] = 
        {
            
            .kind   = ATK_KIND_SIMPLE,
            .triger = ATK_TRIGGER_ON_HIT,

            .stats.damage = 25, 
            .stats.stun_duration = TICKS(60),
            .stats.knockback     = {300.0f, -100.0f},
            .stats.flags  = ATK_FLAG_KNOCKDOWN,
            
            .startup = TICKS(10),
            .active = TICKS(5),
        },    

        .animations[STATE_CROUCH_HEAVY] = ANIM(false, TICKS((4 * 5) + 10),
            FRAME_HURT(TILE_64x96(11), TICKS(5), 24, 88,  8, 48, 36, 36),
            FRAME_HURT(TILE_64x96(12), TICKS(5), 24, 88,  8, 48, 36, 36),
            FRAME_HURT(TILE_64x96(13), TICKS(5), 24, 88,  8, 48, 36, 36),
            FRAME_HIT (TILE_64x96(14), TICKS(5),  8, 88,  8, 48, 36, 36,  32, 56, 40, 16),
            FRAME_HURT(TILE_64x96(14), TICKS(10),  8, 88,  8, 48, 36, 36),
        ),
        .attacks[ATK_ID_CROUCH_HEAVY] = 
        {
            .kind   = ATK_KIND_SIMPLE,
            .triger = ATK_TRIGGER_ON_HIT,
            
            .stats.damage = 25, 
            .stats.stun_duration = TICKS(60),
            .stats.knockback     = {100.0f, 0.0f}, 
            .stats.flags  = ATK_FLAG_KNOCKDOWN | ATK_FLAG_CANT_BLOCK_STANDING,

            .startup = TICKS(15),
            .active = TICKS(5),
        },

        .animations[STATE_AIRBORNE_ATK] = ANIM(false, TICKS(20 + 15),
            FRAME_HURT(TILE_64x96(15), TICKS(20), 24, 64,  8, 32, 44, 44),
            FRAME_HIT (TILE_64x96(15), TICKS(15), 24, 64,  8, 32, 44, 44,  24, 64, 40, 32),
        ),
        .attacks[ATK_ID_AIRBORNE_ATK] = 
        {
            .kind   = ATK_KIND_SIMPLE,
            .triger = ATK_TRIGGER_ON_HIT,
            
            .stats.damage = 10, 
            .stats.stun_duration = TICKS(15),
            .stats.knockback   = {200.0f, 50.0f},
            .stats.recoil      = {200.0f, 0.0f},
            .stats.flags  = ATK_FLAG_KNOCKDOWN | ATK_FLAG_CANT_BLOCK_CROUCHING,
            
            .startup = TICKS(20),
            .active = TICKS(15),
        },

        // ---- 2. ROW -----------------------------------------------------
        .animations[STATE_KNOCKDOWN] = ANIM(false, TICKS(4 * 10),
            FRAME_IMMUNE(TILE_64x96(16), TICKS(10),  32, 88),
            FRAME_IMMUNE(TILE_64x96(17), TICKS(10),  32, 78),
            FRAME_IMMUNE(TILE_64x96(18), TICKS(10),  32, 80),
            FRAME_IMMUNE(TILE_64x96(19), TICKS(10),  32, 88),
        ),

        .animations[STATE_RECOVERY] = ANIM(false, TICKS(3 * 9), 
            FRAME_IMMUNE(TILE_64x96(19), TICKS(9), 32, 88),
            FRAME_IMMUNE(TILE_48x96(10), TICKS(9), 24, 88),
            FRAME_IMMUNE(TILE_48x96(4),  TICKS(9),  24, 88),
        ),

        // ---- COMBOS ---------------------------------------------
        // This animation is the same as: STATE_WALK_BACKWARD
        .animations[STATE_DASH_BACKWARD] = ANIM(false, TICKS(2 * 9), 
            FRAME_HURT(TILE_48x96(11), TICKS(9), 32, 88,  8, 16, 24, 68),
            FRAME_HURT(TILE_48x96(12), TICKS(9), 32, 88,  8, 16, 24, 68),
        ),
        .attacks[ATK_ID_DASH_BACKWARDS] =
        {
            .kind    = ATK_KIND_DASH,
            .triger = ATK_TRIGGER_ON_HIT, 
            .sequence = {{INPUT_PRESSED_LEFT, INPUT_PRESSED_LEFT}, 2},

            .startup = TICKS(9),
            .active = TICKS(0),
        },

        .animations[STATE_DASH_FORWARD] = ANIM(false, TICKS((5 + 5) + (10) + 4),
            FRAME_HURT(TILE_64x96(20), TICKS(5),  24, 88,  0, 24, 6*8, 8*7),
            FRAME_HURT(TILE_64x96(21), TICKS(5),  24, 88,  0, 24, 6*8, 8*7),
            FRAME_HIT (TILE_64x96(21), TICKS(10), 24, 88,  0, 24, 6*8, 8*7, 8*6, 8*7, 8*3, 8*4),
            FRAME_HURT(TILE_64x96(20), TICKS(4),  24, 88,  0, 24, 6*8, 8*7),
        ),
        .attacks[ATK_ID_DASH_FORWARD] =
        {
            .kind    = ATK_KIND_DASH,
            .triger = ATK_TRIGGER_ON_HIT, 
            .sequence = {{INPUT_PRESSED_RIGHT, INPUT_PRESSED_RIGHT}, 2},

            .stats.damage = 10,
            .stats.knockback = {50.0f, -10.0f},
            .stats.recoil    = {150.0f, 0.0f},
            .stats.flags = ATK_FLAG_KNOCKDOWN,
            .stats.stun_duration = TICKS(35),

            .startup = TICKS(10),
            .active  = TICKS(10),
        },
        
        .animations[STATE_COMBO1] = ANIM(false, TICKS(5 + 5 + 7 + 10),
            FRAME_HURT(TILE_64x96(22), TICKS(5), 16, 88,  24, 24, 8*3, 8*8),
            FRAME_HURT(TILE_64x96(23), TICKS(5), 16, 88,  24, 24, 8*3, 8*8),
            FRAME_HIT (TILE_64x96(24), TICKS(7), 16, 88,  24, 24, 8*3, 8*8,  8*5, 8, 32, 24),
            FRAME_HURT(TILE_64x96(24), TICKS(10), 16, 88,  24, 24, 8*3, 8*8),
        ),
        .attacks[ATK_ID_COMBO1] = 
        {
            .kind   = ATK_KIND_SIMPLE,
            .triger = ATK_TRIGGER_ON_HIT,
            .sequence = {{INPUT_PRESSED_DOWN, INPUT_PRESSED_RIGHT, INPUT_PRESSED_LIGHT}, 3},

            .stats.damage = 45, 
            .stats.stun_duration = TICKS(60),
            .stats.knockback = {100.0f, -500.0f},
            .stats.flags  = ATK_FLAG_KNOCKDOWN,

            .stats.hitstop = 7,
            
            .startup = TICKS(10),
            .active = TICKS(7),
        },

        .animations[STATE_COMBO2] = ANIM(false, TICKS(15 + (6 * 8) + 30),
            FRAME_HURT(TILE_64x96(22), TICKS(15), 24, 88,  12, 12, 24, 72),
            FRAME_HIT (TILE_64x96(25), TICKS(8),  24, 88,  12, 12, 24, 72,  8*3, 8*3, 8*5, 8*5),
            FRAME_HIT (TILE_64x96(26), TICKS(8),  24, 88,  12, 12, 24, 72,  8*3, 8*3, 8*5, 8*5),
            FRAME_HIT (TILE_64x96(27), TICKS(8),  24, 88,  12, 12, 24, 72,  8*3, 8*3, 8*5, 8*5),
            FRAME_HIT (TILE_64x96(25), TICKS(8),  24, 88,  12, 12, 24, 72,  8*3, 8*3, 8*5, 8*5),
            FRAME_HIT (TILE_64x96(26), TICKS(8),  24, 88,  12, 12, 24, 72,  8*3, 8*3, 8*5, 8*5),
            FRAME_HIT (TILE_64x96(27), TICKS(8),  24, 88,  12, 12, 24, 72,  8*3, 8*3, 8*5, 8*5),
            FRAME_HURT(TILE_64x96(24), TICKS(30), 24, 88,  12, 12, 24, 72),
        ),
        .attacks[ATK_ID_COMBO2] = 
        {
            .kind   = ATK_KIND_MULTIHIT,
            .triger = ATK_TRIGGER_ON_HIT,
            .sequence = {{INPUT_HOLDING_DOWN, INPUT_PRESSED_RIGHT, INPUT_PRESSED_MEDIUM}, 3},

            .stats.damage = 5, 
            .stats.stun_duration = TICKS(3),
            .stats.knockback = {20.0f, 0.0f},
            .stats.recoil    = {10.0f, 0.0f},
            
            .startup = TICKS(15),
            .active = TICKS(6 * 8),

            .multihit_interval = TICKS(8),
        },

        //.animations[STATE_COMBO3] = ANIM(false, TICKS(),
        //    FRAME_HURT(TILE_64x96(28), TICKS(5), 24, 88,  12, 12, 24, 72),   
        //    FRAME_HURT(TILE_64x96(29), TICKS(5), 24, 88,  12, 12, 24, 72),   
        //    FRAME_HURT(TILE_64x96(30), TICKS(5), 24, 88,  12, 12, 24, 72),   
        //    FRAME_HURT(TILE_64x96(31), TICKS(5), 24, 88,  12, 12, 24, 72),   
        //),
        //.attacks[ATK_ID_COMBO3] = 
        //{
        //    .damage = 50, 
        //    .stun_duration = TICKS(3),
        //    .knockback_x = 100.0f,
        //    
        //    .startup_ticks = TICKS(3 * 5),
        //    .active_ticks = TICKS(5),

        //    .projectile = {.lifetime_ticks = TICKS(60), },

        //    .triger = ATK_TRIGGER_ON_HIT,
        //    .flags  = ATK_FLAG_PROJECTILE,
        //    .sequence = {{INPUT_HOLDING_DOWN, INPUT_PRESSED_RIGHT, INPUT_PRESSED_MEDIUM}, 3}
        //},

        // ---- SIZE 96x96 -------------------------------------------------
        .animations[STATE_POSE_VICTORY] = ANIM(true, TICKS(15 + 45),
            FRAME_IMMUNE(TILE_96x96(0), TICKS(15), 48, 88),
            FRAME_IMMUNE(TILE_96x96(1), TICKS(45), 48, 88),
        ),
        // Same as a knockdown
        .animations[STATE_POSE_DEFEAT] = ANIM(false, TICKS(4 * 10),
            FRAME_IMMUNE(TILE_64x96(16), TICKS(10),  32, 88),
            FRAME_IMMUNE(TILE_64x96(17), TICKS(10),  32, 78),
            FRAME_IMMUNE(TILE_64x96(18), TICKS(10),  32, 80),
            FRAME_IMMUNE(TILE_64x96(19), TICKS(10),  32, 88),
        ),
    },

};
           
#endif /* CHARACTERS_IMPLEMENTATION */

#endif /* !_CHARACTERS_H */
