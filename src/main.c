#include "game.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void game_loop(void);
game_t game = {0};

void game_loop(void)
{
    handle_game_time(&game.time);
    game.time.accumulator += game.time.delta_time;
    while (game.time.accumulator >= game.time.target_frame_s)
    {
        handle_SDL_events(&game);
        machine_update(&game.machine, game.time.target_frame_s);
        game.time.accumulator -= game.time.target_frame_s;
    }

    machine_render(&game.machine, &game.renderer);
    return;
}

int main(int argc, char **argv) 
{
    (void)argc;
    (void)argv;
    
    if (!init_game(&game)) 
        no_game(&game, EXIT_FAILURE);
    
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(game_loop, 0, 1);
#else
    while (game.running) { game_loop(); }
#endif
    
    game_loop();
    
    no_game(&game, EXIT_SUCCESS);
    return EXIT_SUCCESS;
}
