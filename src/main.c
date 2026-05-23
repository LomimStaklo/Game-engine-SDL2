#include "game.h"
#include "state.h"

int main(int argc, char **argv) 
{
    (void)argc;
    (void)argv;

    game_t game = {0};
    
    if (!(init_game(&game))) 
        no_game(&game, EXIT_FAILURE);
    
    game_state_t st = {0};
    state_init(&st, &game.renderer, &game.players[0], &game.players[1]);
    
    while (game.running)
    {
        handle_game_time(&game.time);
        handle_SDL_events(&game);
        
        state_update(&st, game.time.delta_time);
        state_render(&st, &game.renderer);
    }
    
    no_game(&game, EXIT_SUCCESS);
    return EXIT_SUCCESS;
}
