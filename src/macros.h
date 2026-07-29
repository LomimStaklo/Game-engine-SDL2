#ifndef _MACROS_H
#define _MACROS_H

#include <stdint.h>
#include <assert.h>

// Checks range includeing first number "from" but excludeing last "to" 
#define is_in_range(from, to, value) (((from) <= (value)) && ((value) < (to)))
#define TO_STR(name) #name
#define lenghtof(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define for_range_i(count) for (uint32_t i = 0; i < (count); i++)
#define for_range_j(count) for (uint32_t j = 0; j < (count); j++)
#define str_bool(expr) (expr) ? "true" : "false"  

// Vector implementation

typedef struct vec2i_t { int32_t x, y; } vec2i_t;
typedef struct vec2f_t { float x, y; }   vec2f_t;

static inline vec2i_t vec2i(int32_t x, int32_t y) { return (vec2i_t){x, y}; }
static inline vec2f_t vec2f(float x, float y)     { return (vec2f_t){x, y}; }

static inline vec2i_t vec2i_add(vec2i_t vec1, vec2i_t vec2) { return (vec2i_t){ vec1.x + vec2.x, vec1.y + vec2.y }; }
static inline vec2i_t vec2i_sub(vec2i_t vec1, vec2i_t vec2) { return (vec2i_t){ vec1.x - vec2.x, vec1.y - vec2.y }; }
static inline vec2i_t vec2i_mul(vec2i_t vec1, vec2i_t vec2) { return (vec2i_t){ vec1.x * vec2.x, vec1.y * vec2.y }; }
static inline vec2i_t vec2i_div(vec2i_t vec1, vec2i_t vec2) { return (vec2i_t){ vec1.x / vec2.x, vec1.y / vec2.y }; }

static inline vec2f_t vec2f_add(vec2f_t vec1, vec2f_t vec2) { return (vec2f_t){ vec1.x + vec2.x, vec1.y + vec2.y }; }
static inline vec2f_t vec2f_sub(vec2f_t vec1, vec2f_t vec2) { return (vec2f_t){ vec1.x - vec2.x, vec1.y - vec2.y }; }
static inline vec2f_t vec2f_mul(vec2f_t vec1, vec2f_t vec2) { return (vec2f_t){ vec1.x * vec2.x, vec1.y * vec2.y }; }
static inline vec2f_t vec2f_div(vec2f_t vec1, vec2f_t vec2) { return (vec2f_t){ vec1.x / vec2.x, vec1.y / vec2.y }; }

/** 
 * Compile time assert
 * \param val the value that gets returned
 * \param expr the assertion expresion
 * \returns an integer, or a compile time error  
 */
#define inline_static_assert(val, expr) ((val) * sizeof(char [(expr) ? 1 : -1])) 

/** 
 * Index based atlas access that assumes the origin at (0, 0)
 * \param idx index of a tile
 * \param tile_w width of a tile
 * \param tile_h height of a tile
 * \param atlas_columns the amount of columns in a tile atlas
 * \returns Rectangle (SDL_Rect) of the tile in a atlas 
 */  
#define tile_from_atlas(idx, tile_w, tile_h, atlas_columns) \
    {                                                       \
        ((idx) % (atlas_columns)) * (tile_w),               \
        ((idx) / (atlas_columns)) * (tile_h),               \
        (tile_w),                                           \
        (tile_h)                                            \
    }

/** 
 * Index based atlas access with custom origin point 
 * \param origin_x origin of x
 * \param origin_y origin of y
 * \param idx index of a tile
 * \param tile_w width of a tile
 * \param tile_h height of a tile
 * \param atlas_columns the amount of columns in a tile atlas
 * \returns Rectangle (SDL_Rect) of the tile in a atlas 
 */
#define tile_from_atlas_xy(idx, origin_x, origin_y, tile_w, tile_h, atlas_columns) \
    {                                                                              \
        (origin_x) + (((idx) % (atlas_columns)) * (tile_w)),                       \
        (origin_y) + (((idx) / (atlas_columns)) * (tile_h)),                       \
        (tile_w),                                                                  \
        (tile_h)                                                                   \
    }

#endif /* !_MACROS_H */
