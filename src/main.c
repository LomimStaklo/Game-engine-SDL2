#include "game.h"
#include "systems.h"

// printf("curs:%d\n", SDL_ShowCursor(SDL_DISABLE)); SDL_GetScancodeName(game.event.key.keysym.scancode)
static inline void render_display(game_t *game)
{
    SDL_RenderPresent(game->renderer);
    SDL_RenderClear(game->renderer);
}

int main(__attribute__((unused)) int argc, __attribute__((unused)) char **argv) 
{
    game_t game = {0};
    if ( !(init_game(&game)) )  no_game(&game, EXIT_FAILURE);
    if ( !(load_media(&game)) ) no_game(&game, EXIT_FAILURE);

    SDL_Rect txt_rect = { .x = 0, .y = 0, .w = 20, .h = 20 };
    SDL_Color red = { .r = 255, .a = 255 };
    char info_str[256] = "";

    while (game.running)
    {
        handle_delta_time(&game.time);
        
        while (SDL_PollEvent(&game.event))
            handle_events(&game);

        snprintf(info_str, sizeof(info_str), "w:%d h:%d\nmouse:(%d, %d)",
        game.position_win.w, game.position_win.h, game.mouse.x, game.mouse.y);
        
        txt_rect.x = (game.position_win.w - txt_rect.w) / 2;
        txt_rect.y = (game.position_win.h - txt_rect.h) / 2;

        render_text(game.renderer, &game.media.atlas, info_str, txt_rect, red);
        
        render_display(&game);
    }

    printf("frame_counter:%u\n", game.time.frame_counter);
    no_game(&game, EXIT_SUCCESS);
    
    return EXIT_SUCCESS;
}
