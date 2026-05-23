#define PLAYER_IMPLEMENTATION
#define STATE_IMPLEMENTATION
#define RENDERER_IMPLEMENTATION
#define CHARACTERS_IMPLEMENTATION
#define FAJTER_IMPLEMENTATION
#define MATCH_IMPLEMENTATION
#define ASSETS_IMPLEMENTATION
#include "assets.h"
#include "state.h"
#include "match.h"
#include "game.h"

#include "loging.h"
#include "macros.h"
#include <SDL2/SDL_image.h>

#define INIT_FLAGS  (SDL_INIT_VIDEO | SDL_INIT_TIMER  | SDL_INIT_EVENTS)
#define IMAGE_FLAGS (IMG_INIT_JPG | IMG_INIT_PNG)

bool init_game(game_t *game)
{
    memset((void *)game, 0, sizeof(game_t));

    if (SDL_Init(INIT_FLAGS) < 0) { 
        game_log( "ERROR", "SDL init: %s", SDL_GetError() );
        return false;
    } 
    if (IMG_Init(IMAGE_FLAGS) != IMAGE_FLAGS) { 
        game_log( "ERROR", "IMG init: %s", SDL_GetError() );
        return false;
    }

    if (!init_renderer(&game->renderer)) {
        game_log( "ERROR", "SDL renderer: %s", SDL_GetError() );
        return false; 
    }
    
    if (!init_players(game->players, &game->input)) {
        game_log( "ERROR", "Keys not initialised count: %d", BUTTON_COUNT);
        return false;
    }

    // Icon loading
    SDL_Surface *ico = IMG_Load(IMAGE_PATH("ui_icon.png"));
    if (ico)
    {
        SDL_SetWindowIcon(game->renderer.sdl_window, ico);
        SDL_FreeSurface(ico);
    }

    game_time_init(&game->time, 60); // FPS 60 
    game->input.mouse.is_active = SDL_ShowCursor(SDL_ENABLE);
    game->running = true; 
    return true;
}

void no_game(game_t *game, int32_t exit_code)
{
    //unload_media(game);
    destroy_renderer(&game->renderer);

    IMG_Quit();
    SDL_Quit();
    exit(exit_code); 
}

void game_time_init(game_time_t *time, int32_t target_fps)
{
    time->frequency       = SDL_GetPerformanceFrequency();
    time->frame_start     = SDL_GetPerformanceCounter();
    time->delta_time      = 0.0f;
    time->target_frame_s  = 1.0f / (float)target_fps;
    time->target_frame_ms = (uint32_t)(1000 / target_fps);
}

void handle_game_time(game_time_t *time)
{
    uint64_t now     = SDL_GetPerformanceCounter();
    uint64_t elapsed = now - time->frame_start;

    // Convert to seconds using cached frequency
    time->delta_time = (float)elapsed / (float)time->frequency;

    // Delta time cap: avoid spiral of death on lag spikes
    if (time->delta_time > 0.05f)
        time->delta_time = 0.05f;

    // Sleep for the bulk of remaining frame time
    float remaining_s = time->target_frame_s - time->delta_time;
    if (remaining_s > 0.002f) // Only sleep if > 2ms remain (avoid oversleeping)
        SDL_Delay((uint32_t)((remaining_s - 0.001f) * 1000.0f));

    // Busy-wait for the last ~1ms to hit the frame target precisely
    do {
        now = SDL_GetPerformanceCounter();
        elapsed = now - time->frame_start;
    } while ((float)elapsed / (float)time->frequency < time->target_frame_s);

    // Recalculate true delta after busy-wait
    time->delta_time = (float)elapsed / (float)time->frequency;
    if (time->delta_time > 0.05f)
        time->delta_time = 0.05f;

    time->frame_start = SDL_GetPerformanceCounter();
}

void handle_SDL_events(game_t *game)
{
    // Input buffer clearing 
    for_range_i(SDL_NUM_SCANCODES)
    {
        game->input.keys[i].pressed  = false;
        game->input.keys[i].released = false;
    }
    for_range_i(MOUSE_BUTTON_COUNT)
    {
        game->input.mouse.buttons[i].pressed  = false;
        game->input.mouse.buttons[i].released = false;
    }

    // Polling events
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_KEYDOWN:
            {
                if (!event.key.repeat) 
                {
                    game->input.keys[event.key.keysym.scancode].holding = true;
                    game->input.keys[event.key.keysym.scancode].pressed = true;
                }
            } break;

            case SDL_KEYUP: 
            {
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) game->running = false; 
                if (event.key.keysym.scancode == SDL_SCANCODE_F11)
                {
                    if (SDL_GetWindowFlags(game->renderer.sdl_window) & SDL_WINDOW_FULLSCREEN_DESKTOP)
                        SDL_SetWindowFullscreen(game->renderer.sdl_window, 0);
                    else
                        SDL_SetWindowFullscreen(game->renderer.sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                }

                game->input.keys[event.key.keysym.scancode].holding  = false;
                game->input.keys[event.key.keysym.scancode].released = true;
            } break;

            case SDL_MOUSEMOTION:
                if (game->input.mouse.is_active) 
                {
                    game->input.mouse.x = event.motion.x; 
                    game->input.mouse.y = event.motion.y; 
                }
                break;
            
            case SDL_MOUSEBUTTONDOWN:
                if (game->input.mouse.is_active) 
                {
                    mouse_button_t button = event.button.button - 1;
                    game->input.mouse.buttons[button].holding = true;
                    game->input.mouse.buttons[button].pressed = true;
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (game->input.mouse.is_active) 
                {
                    mouse_button_t button = event.button.button - 1;
                    game->input.mouse.buttons[button].holding  = false;
                    game->input.mouse.buttons[button].released = true;
                }
                break;

            case SDL_QUIT: 
                game->running = false;
                break;

            default: break;
        }
    }
}

