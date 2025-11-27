#ifndef _GAME_H
#define _GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define _LOG_FILE stderr
#define RET_VOID

#define _PP_RED      "\033[31m" 
#define _PP_AEND     "\033[0m" 

#define log_error_(__expr_, __return_, __fmt_, ...) do {        \
    if (__expr_) {                                              \
        fprintf(_LOG_FILE, _PP_RED"[ERROR]"_PP_AEND" %s:%d:%s(): " __fmt_ "\n", \
        __FILE__, __LINE__, __func__, ##__VA_ARGS__);           \
        return __return_; }                                     \
    } while (0)

typedef enum {
    LAYER_BACKGROUND,
    LAYER_BACKGROUND2,
    LAYER_ENTITY,
    LAYER_TEXT,
    LAYER_COUNT
} render_layer_t;

typedef struct {
    SDL_Texture *texture;
    SDL_Rect srcrect, dstrect;
    render_layer_t layer;
} render_tex_t;

typedef struct {
    SDL_Rect dstrect;
    SDL_Color color;
    char *text;
    struct {
        uint16_t w, h;
    } chr_size;
} textbox_t;

typedef struct {
    SDL_Texture *texture;
    int32_t font_height;
    struct glyph {
        SDL_Rect src;
        //int32_t advance; 
    } glyphs[128];
} font_atlas_t;

typedef struct {
    font_atlas_t atlas;
    SDL_Texture *background_img;
} media_t;

typedef struct {
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Event event;
    media_t media;
    
    const uint8_t *keyboard;
    bool running;
    struct {
        int32_t w, h;
    } position_win;
    struct {
        bool is_active;
        uint32_t button;
        int32_t x, y;
    } mouse;
    struct game_time {
        uint32_t frame_elapsed;
        uint32_t frame_start;
        uint32_t frame_counter;
        float frame_rate;
    } time;
} game_t;

bool init_game(game_t *game);
void no_game(game_t *game, int exit_code);
bool load_media(game_t *game);

font_atlas_t create_atlas(SDL_Renderer *renderer, const char *font_path);
void render_text(SDL_Renderer *renderer, font_atlas_t *atlas, char *text, SDL_Rect text_rect, SDL_Color col);

#endif /* _GAME_H */
