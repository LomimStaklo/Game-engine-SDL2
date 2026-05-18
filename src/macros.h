#ifndef _MACROS_H
#define _MACROS_H

#include <stdint.h>
#include <assert.h>

// Checks range includeing first number "from" but excludeing last "to" 
#define is_in_range(from, to, value) (((from) <= (value)) && ((value) < (to)))
#define to_str(name) #name
#define lenghtof(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define for_range_i(count) for (uint32_t i = 0; i < (count); i++)
#define for_range_j(count) for (uint32_t j = 0; j < (count); j++)

/** 
 * Compile time assert
 * \param val the value that gets returned
 * \param expr the assertion expresion
 * \returns an integer or a compile time error  
 */
#define inline_static_assert(val, expr) (sizeof(char [(expr) ? -1 : (val)])) 

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
