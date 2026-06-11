#ifndef _MACHINE_H
#define _MACHINE_H

// Header only file!
// For implementation you will need to define:
// #define MACHINE_IMPLEMENTATION

#include <stdint.h>
#include <stdbool.h>
#include "match.h"

struct renderer_t;
struct player_t;

typedef enum machine_state_t 
{
    GAME_STATE_MENU = 0,
    GAME_STATE_MATCH,
} machine_state_t;

typedef struct machine_t
{
    // For machine machine
    machine_state_t curr_state;
    machine_state_t next_state;
    float state_timer;
    
    // Transition
    bool  transitioning;
    float transition_timer;
    float transition_duration; 
    
    // GAME_STATE_MENU
    int32_t idx_p1, idx_p2;
    
    // GAME_STATE_MATCH 
    match_t match;
} machine_t;

void machine_init(machine_t *machine, struct renderer_t *renderer, struct player_t *p1, struct player_t *p2);
void machine_update(machine_t *machine, float delta_time);
void machine_render(machine_t *machine, struct renderer_t *renderer);

#ifdef MACHINE_IMPLEMENTATION
#include "renderer.h"
#include "player.h"
#include "characters.h" 

static void machine_perform_switch(machine_t *machine)
{
    if (machine->curr_state != machine->next_state)
    {
        machine->curr_state = machine->next_state;
        machine->state_timer = 0.0f;
    }
}

static void machine_queue_state(machine_t *machine, machine_state_t next)
{
    machine->next_state = next; 
}

static void machine_begin_transition(machine_t *machine, float duration)
{
    machine->transitioning = true;
    machine->transition_timer = 0.0f;
    machine->transition_duration = duration;
}

void machine_init(machine_t *machine, renderer_t *renderer, player_t *p1, player_t *p2)
{
    machine->curr_state = GAME_STATE_MENU;
    machine->next_state = GAME_STATE_MENU;
    
    p1->fighter = character_get(renderer, CHARACTER_BOKE, 0);
    p2->fighter = character_get(renderer, CHARACTER_BOKE, 1);

    match_textures_t texs = 
    {
        .ui    = asset_get_texture(ASSET_UI_GAME_BAR),
        .stage = asset_get_texture(ASSET_STAGE_CAVA)
    }; 
    
    machine->match = match_start(90.0f, 3, p1, p2, &texs);
}

void machine_update(machine_t *machine, float delta_time)
{
    machine->state_timer += delta_time;
    
    if (machine->transitioning) 
    {
        machine->transition_timer += delta_time;
        float half = machine->transition_duration * 0.5f;

        if (machine->transition_timer >= half)
            machine_perform_switch(machine);

        if (machine->transition_timer >= machine->transition_duration)
            machine->transitioning = false;
    }

    switch (machine->curr_state)
    {
        case GAME_STATE_MENU:
            if (!machine->transitioning)
            {
                machine_queue_state(machine, GAME_STATE_MATCH);
                machine_begin_transition(machine, 5.0f);
            }
        break;

        case GAME_STATE_MATCH:
            match_update(&machine->match, delta_time);

            if (machine->match.state == MATCH_STATE_EXIT && (!machine->transitioning))
            {
                machine_queue_state(machine, GAME_STATE_MENU);
                machine_begin_transition(machine, 5.0f); 
            }
            
        break;
    }
}

void machine_render(machine_t *machine, renderer_t *renderer)
{
    renderer_start_drawing(renderer);
    
    switch (machine->curr_state)
    {
        case GAME_STATE_MENU: 
            renderer_draw_rect(renderer, LAYER_BACKGROUND, NULL, COLOR_WHITE, true);
        break;
        case GAME_STATE_MATCH:
            match_render(&machine->match, renderer);
        break;
    }
    
    if (machine->transitioning) 
    {
        float t = machine->transition_timer / machine->transition_duration;
        uint8_t alpha = t < 0.5f 
            ? (uint8_t)(t * 2.0f * 255.0f)
            : (uint8_t)((1.0f - (t - 0.5f) * 2.0f) * 255.0f);
        
        SDL_Color trans = {0, 0, 0, alpha};
        renderer_draw_rect(renderer, LAYER_UI2, NULL, trans, true);
    }

    renderer_present(renderer);
}

#endif // MACHINE_IMPLEMENTATION
#endif // !_MACHINE_H
