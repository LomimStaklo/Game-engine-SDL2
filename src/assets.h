#ifndef _ASSETS_H
#define _ASSETS_H

// Header only file!
// For implementation you will need to define:
// #define ASSETS_IMPLEMENTATION

#include <stdint.h>
#include <stdbool.h>

struct renderer_t;

typedef enum asset_type_t 
{
    ASSET_TYPE_IMAGE,
    ASSET_TYPE_SOUND
} asset_type_t;

typedef struct asset_t
{
    const char *path;
    asset_type_t type;

    union { int32_t texture, sound; } handle;
    
    size_t size;
    size_t offset;
    bool loaded;
} asset_t;


#define IMAGE_PATH(file) "assets/images/"file
#define SOUND_PATH(file) "assets/audio/"file

/**
 * X macro list with all game assets
 * Macro declaration:
 * #define X(name, type, file) 
 */
#define ASSET_XLIST \
X(ATLAS_BOKE,  ASSET_TYPE_IMAGE, "atlas_boke.png") \
X(STAGE_CAVA,  ASSET_TYPE_IMAGE, "stage_cava.jpg") \
X(UI_GAME_BAR, ASSET_TYPE_IMAGE, "ui_bar.png") \
X(UI_FONT,     ASSET_TYPE_IMAGE, "ui_font.png") \
X(UI_ICON,     ASSET_TYPE_IMAGE, "ui_icon.png")

typedef enum asset_name_t
{
#define X(name, type, filename) ASSET_ ## name,
    ASSET_XLIST
#undef X
    ASSET_COUNT
} asset_name_t;

/** 
 * Macro defining if the assets are baked in  
*/

extern asset_t global_assets[ASSET_COUNT];
#ifdef ASSETS_BAKED
extern const uint8_t assets_baked_data[];
#endif // ASSETS_BAKED

bool asset_load(asset_t *asset, struct renderer_t *renderer);
void asset_unload(asset_t *asset, struct renderer_t *renderer);
int32_t asset_get_texture(asset_name_t name);

bool asset_load_all(struct renderer_t *renderer);
void asset_unload_all(struct renderer_t *renderer);

#ifdef ASSETS_IMPLEMENTATION
#include "renderer.h"

bool asset_load(asset_t *asset, renderer_t *renderer)
{
    if (asset->loaded) return true;
    
    assert(asset->type != ASSET_TYPE_SOUND && "No suport for sounds right now");
    
    if (asset->type == ASSET_TYPE_IMAGE)
    {
        texture_handle_t handle = INVALID_TEXTURE_HANDLE;
        
#ifdef ASSETS_BAKED 
        const uint8_t *data = assets_baked_data + asset->offset;
        handle = renderer_load_texture_from_mem(renderer, data, asset->size);
        assert(handle != INVALID_TEXTURE_HANDLE && "Couldn't load baked image");
#else
        handle = renderer_load_texture(renderer, asset->path);
        assert(handle != INVALID_TEXTURE_HANDLE && "Couldn't load image form assets/ floder");
#endif
        asset->handle.texture = handle;
        asset->loaded = (handle != INVALID_TEXTURE_HANDLE);
    }
    
    return asset->loaded;
}

void asset_unload(asset_t *asset, renderer_t *renderer)
{
    if (!asset->loaded) return;

    if (asset->type == ASSET_TYPE_IMAGE)
    {
        renderer_unload_texture(renderer, asset->handle.texture);
        asset->loaded = false;
    }
}

int32_t asset_get_texture(asset_name_t name)
{
    if (!is_in_range(0, ASSET_COUNT, name)) 
        return INVALID_TEXTURE_HANDLE; 
        
    asset_t *ass = &global_assets[name];
    if (!ass->loaded) return INVALID_TEXTURE_HANDLE;
    
    return ass->handle.texture;
}

bool asset_load_all(renderer_t *renderer)
{
    for_range_i(ASSET_COUNT)
    {
        if (!asset_load(&global_assets[i], renderer))
            return false;
    }
    return true;
}

void asset_unload_all(renderer_t *renderer)
{
    for_range_i(ASSET_COUNT)
    {
        asset_unload(&global_assets[i], renderer);
    }
}

/** If the are baked then it incldes the raw bytes of every asset */
#ifdef ASSETS_BAKED 
#include "assets_baked.h"
#else
asset_t global_assets[ASSET_COUNT] = {
    #define X(name, type_, file) \
    [ASSET_ ## name] = {.type = type_, .path = (type_ == ASSET_TYPE_IMAGE) ? IMAGE_PATH(file) : file},
        ASSET_XLIST
    #undef X
};
#endif

#endif // ASSETS_IMPLEMENTATION

#endif // !_ASSETS_H

