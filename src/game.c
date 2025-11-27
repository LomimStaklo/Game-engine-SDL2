#include "game.h"

#define WIN_TITLE       "Blidinje"
#define WIN_HIGHT       600
#define WIN_WIDTH       800
#define WIN_FLAGS       (SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE) // | SDL_WINDOW_BORDERLESS
#define IMAGE_FLAGS     (IMG_INIT_JPG)
#define RENDERER_FLAGS  (SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)

#define FONT_PTSIZE     70
#define FPS             60

bool init_game(game_t *game)
{
    log_error_( SDL_Init(SDL_INIT_EVERYTHING) < 0   , false, "SDL init: %s", SDL_GetError() );
    log_error_( IMG_Init(IMAGE_FLAGS) != IMAGE_FLAGS, false, "IMG init: %s", SDL_GetError() );
    log_error_( TTF_Init()                          , false, "TTF init: %s", SDL_GetError() ); 

    game->window = SDL_CreateWindow
    (
        WIN_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIN_WIDTH, WIN_HIGHT,
        WIN_FLAGS
    );
    log_error_( !game->window, false, "SDL window: %s", SDL_GetError() );  
    
    game->renderer = SDL_CreateRenderer(game->window, -1, RENDERER_FLAGS);
    log_error_( !game->renderer, false, "SDL renderer: %s", SDL_GetError() ); 
    
    game->keyboard = SDL_GetKeyboardState(NULL);
    game->mouse.is_active = SDL_ShowCursor(SDL_ENABLE);
    game->time.frame_rate = 1000.0f / FPS;
    game->position_win.w = WIN_WIDTH;
    game->position_win.h = WIN_HIGHT;
    game->running = true; 
    return true;
}

bool load_media(game_t *game)
{
    game->media.atlas = create_atlas(game->renderer, "./fonts/jjba.ttf"); // FreeSansBold.ttf Jaldi-Regular
    return true;
}

void unload_media(game_t *game)
{
    SDL_DestroyTexture(game->media.atlas.texture);
}

void no_game(game_t *game, int exit_code)
{
    unload_media(game);
    SDL_DestroyRenderer(game->renderer);
    SDL_DestroyWindow(game->window);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    exit(exit_code);
}

font_atlas_t create_atlas(SDL_Renderer *renderer, const char *font_path) 
{
    font_atlas_t atlas = {0};

    TTF_Font *font = TTF_OpenFont(font_path, FONT_PTSIZE);
    log_error_( !font, atlas, "TTF font: %s", SDL_GetError() );
    
    atlas.font_height = TTF_FontHeight(font);

    const int32_t glyph_rows = 16;
    const int32_t glyph_size = atlas.font_height + 4;
    const int32_t tex_size = glyph_rows * glyph_size;

    SDL_Surface *atlas_surf = SDL_CreateRGBSurface(0, tex_size, tex_size, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_FillRect(atlas_surf, NULL, SDL_MapRGBA(atlas_surf->format, 0, 0, 0, 0));

    for (unsigned char chr = 32; chr < 128; chr++) 
    {
        char str[2] = { chr, '\0' };
        SDL_Surface *glyph_surf = TTF_RenderText_Blended(font, str, (SDL_Color) {255, 255, 255, 255});
        if ( !glyph_surf ) { continue; }

        int32_t x = ((chr - 32) % glyph_rows) * glyph_size;
        int32_t y = ((chr - 32) / glyph_rows) * glyph_size;
        SDL_Rect dst = { x, y, glyph_surf->w, glyph_surf->h };
        SDL_BlitSurface(glyph_surf, NULL, atlas_surf, &dst);

        atlas.glyphs[chr].src = dst;
        //TTF_GlyphMetrics(font, chr, NULL, NULL, NULL, NULL, &atlas.glyphs[chr].advance);
 
        SDL_FreeSurface(glyph_surf);
    }
    atlas.texture = SDL_CreateTextureFromSurface(renderer, atlas_surf);
    //IMG_SavePNG(atlas_surf, "./images/fatlas.png");
    SDL_FreeSurface(atlas_surf);
    
    TTF_CloseFont(font);
    log_error_( !atlas.texture, (font_atlas_t) {0}, "TTF atlas texture: %s", SDL_GetError() );

    return atlas;
}

void render_text(SDL_Renderer *renderer, font_atlas_t *atlas, char *text, SDL_Rect text_rect, SDL_Color col) 
{
    int32_t x_dst = text_rect.x;
    int32_t y_dst = text_rect.y;

    for (unsigned char *str = (unsigned char *)text; *str; str++)
    {
        unsigned char chr = *str;
        if (chr < 32 || chr >= 128) {
            if (chr == '\n') {
                y_dst += text_rect.h;
                x_dst = text_rect.x; 
            }
            continue;
        }

        struct glyph *gly = &atlas->glyphs[chr];
        SDL_Rect dst = { x_dst, y_dst, text_rect.w, text_rect.h };
        
        SDL_SetTextureColorMod(atlas->texture, col.r, col.g, col.b);
        SDL_RenderCopy(renderer, atlas->texture, &gly->src, &dst);
        SDL_SetTextureColorMod(atlas->texture, 255, 255, 255);
        
        x_dst += text_rect.w;
    }
}
