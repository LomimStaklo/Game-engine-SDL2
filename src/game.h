#ifndef _GAME_H
#define _GAME_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdbool.h>
#include "player.h"
#include "renderer.h"
#include "machine.h"

typedef struct game_time_t
{
    uint64_t frequency;       // Cache frequency (it's constant)
    uint64_t frame_start;     // Use uint64_t for performance counter
    uint32_t target_frame_ms; // Keep for SDL_Delay fallback
    float target_frame_s;     // Target in seconds (more natural)
    float delta_time;         
    float accumulator; 
} game_time_t;

typedef struct game_t 
{
    input_t input;
    renderer_t renderer;
    game_time_t time;
    machine_t machine;

    player_t p1, p2;
    bool running;
} game_t;

bool init_game(game_t *game);
void no_game(game_t *game, int32_t exit_code);

void game_time_init(game_time_t *time, int32_t target_fps);
void handle_game_time(game_time_t *time);
void handle_SDL_events(game_t *game);

#endif /* !_GAME_H */
