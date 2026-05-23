#ifndef _STATES_H
#define _STATES_H

// Header only file!
// For implementation you will need to define:
// #define STATE_IMPLEMENTATION

#include <stdint.h>
#include <stdbool.h>
#include "match.h"

struct renderer_t;
struct player_t;

typedef enum game_state_id_t 
{
    GAME_STATE_MENU = 0,
    GAME_STATE_MATCH,
} game_state_id_t;

typedef struct game_state_t
{
    // For state machine
    game_state_id_t curr_state;
    game_state_id_t next_state;
    float state_timer;
    
    // Transition
    bool  transitioning;
    float transition_timer;
    float transition_duration; 
    
    // GAME_STATE_MENU
    int32_t idx_p1, idx_p2;
    
    // GAME_STATE_MATCH 
    match_t match;
} game_state_t;


void state_init(game_state_t *state, struct renderer_t *renderer, struct player_t *p1, struct player_t *p2);
void state_update(game_state_t *state, float delta_time);
void state_render(game_state_t *state, struct renderer_t *renderer);

#ifdef STATE_IMPLEMENTATION
#include "renderer.h"
#include "player.h"
#include "characters.h" 

static void state_perform_switch(game_state_t *state)
{
    if (state->curr_state != state->next_state)
    {
        state->curr_state = state->next_state;
        state->state_timer = 0.0f;
    }
}

static void state_queue_state(game_state_t *state, game_state_id_t next)
{
    state->next_state = next; 
}

static void state_begin_transition(game_state_t *state, float duration)
{
    state->transitioning = true;
    state->transition_timer = 0.0f;
    state->transition_duration = duration;
}

void state_init(game_state_t *state, renderer_t *renderer, player_t *p1, player_t *p2)
{
    state->curr_state = GAME_STATE_MENU;
    state->next_state = GAME_STATE_MENU;

    fighter_t boke = character_get("boke");
    
    p1->fighter = boke;
    p2->fighter = boke;
    
    state->match = match_start(90.0f, 3, p1, p2);
}

void state_update(game_state_t *state, float delta_time)
{
    state->state_timer += delta_time;
    
    if (state->transitioning) 
    {
        state->transition_timer += delta_time;
        float half = state->transition_duration * 0.5f;

        if (state->transition_timer >= half)
            state_perform_switch(state);

        if (state->transition_timer >= state->transition_duration)
            state->transitioning = false;
    }

    switch (state->curr_state)
    {
        case GAME_STATE_MENU:
            if (!state->transitioning)
            {
                state_queue_state(state, GAME_STATE_MATCH);
                state_begin_transition(state, 5.0f);
            }
        break;

        case GAME_STATE_MATCH:
            match_update(&state->match, delta_time);

            if (state->match.state == MATCH_STATE_EXIT && (!state->transitioning))
            {
                state_queue_state(state, GAME_STATE_MENU);
                state_begin_transition(state, 5.0f); 
            }
            
        break;
    }
}

void state_render(game_state_t *state, renderer_t *renderer)
{
    renderer_start_drawing(renderer);
    
    switch (state->curr_state)
    {
        case GAME_STATE_MENU: 
            renderer_draw_rect(renderer, LAYER_BACKGROUND, NULL, COLOR_WHITE, true);
        break;
        case GAME_STATE_MATCH:
            match_render(&state->match, renderer);
        break;
    }
    
    if (state->transitioning) 
    {
        float t = state->transition_timer / state->transition_duration;
        uint8_t alpha = t < 0.5f 
            ? (uint8_t)(t * 2.0f * 255.0f)
            : (uint8_t)((1.0f - (t - 0.5f) * 2.0f) * 255.0f);
        
        SDL_Color trans = {0, 0, 0, alpha};
        renderer_draw_rect(renderer, LAYER_UI2, NULL, trans, true);
    }

    renderer_present(renderer);
}

    
#endif // STATE_IMPLEMENTATION
#endif // !_STATES_H
