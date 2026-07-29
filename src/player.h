#ifndef _PLAYER_H
#define _PLAYER_H 

// Header only file! 
// For implementation you will need to define:
// #define PLAYER_IMPLEMENTATION

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include "fajter.h"

// =============
//  DECLARATION
// =============

typedef struct button_state_t
{
    bool holding;
    bool pressed;
    bool released;
} button_state_t;

typedef enum input_actions_t
{
    INPUT_NONE = 0,

    INPUT_HOLDING_UP      = 1 << 0,
    INPUT_HOLDING_DOWN    = 1 << 1,
    INPUT_HOLDING_RIGHT   = 1 << 2,
    INPUT_HOLDING_LEFT    = 1 << 3,
    INPUT_HOLDING_LIGHT   = 1 << 4,
    INPUT_HOLDING_MEDIUM  = 1 << 5,
    INPUT_HOLDING_HEAVY   = 1 << 6,
    INPUT_HOLDING_BLOCK   = INPUT_HOLDING_LIGHT | INPUT_HOLDING_LEFT,
     
    INPUT_PRESSED_UP      = (INPUT_HOLDING_UP     << 8),
    INPUT_PRESSED_DOWN    = (INPUT_HOLDING_DOWN   << 8),
    INPUT_PRESSED_RIGHT   = (INPUT_HOLDING_RIGHT  << 8),
    INPUT_PRESSED_LEFT    = (INPUT_HOLDING_LEFT   << 8),
    INPUT_PRESSED_LIGHT   = (INPUT_HOLDING_LIGHT  << 8),
    INPUT_PRESSED_MEDIUM  = (INPUT_HOLDING_MEDIUM << 8),
    INPUT_PRESSED_HEAVY   = (INPUT_HOLDING_HEAVY  << 8),
    INPUT_PRESSED_BLOCK   = (INPUT_HOLDING_BLOCK  << 8),
    
    INPUT_RELEASED_UP     = (INPUT_HOLDING_UP     << 16), 
    INPUT_RELEASED_DOWN   = (INPUT_HOLDING_DOWN   << 16), 
    INPUT_RELEASED_RIGHT  = (INPUT_HOLDING_RIGHT  << 16), 
    INPUT_RELEASED_LEFT   = (INPUT_HOLDING_LEFT   << 16), 
    INPUT_RELEASED_LIGHT  = (INPUT_HOLDING_LIGHT  << 16), 
    INPUT_RELEASED_MEDIUM = (INPUT_HOLDING_MEDIUM << 16),
    INPUT_RELEASED_HEAVY  = (INPUT_HOLDING_HEAVY  << 16), 
    INPUT_RELEASED_BLOCK  = (INPUT_HOLDING_BLOCK  << 16),
} input_actions_t;

typedef enum button_actions_t
{
    BUTTON_UP = 0,
    BUTTON_DOWN,
    BUTTON_RIGHT,
    BUTTON_LEFT,
    BUTTON_LIGHT,
    BUTTON_MEDIUM,
    BUTTON_HEAVY,
    BUTTON_COUNT,
} button_actions_t;

typedef enum mouse_button_t 
{
    MOUSE_BUTTON_LEFT = 0,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_COUNT,
} mouse_button_t;

typedef struct input_t
{
    button_state_t keys[SDL_NUM_SCANCODES];
    struct 
    {
        button_state_t buttons[MOUSE_BUTTON_COUNT];
        int32_t x, y;
        bool is_active;
    } mouse;
} input_t;

typedef struct player_t
{
    fighter_t fighter; 
    button_state_t *keybinds[BUTTON_COUNT];

    // recoded moves going form oldest[0] -> newest[len] but the sequence is the other way around
    input_actions_t input_history[16];
    float           input_timestamps[16]; // Timestamp history of inputs
    float           input_timer; // Timestamp of the newest input
} player_t;

bool init_players(input_t *input, player_t *p1, player_t *p2);
input_actions_t player_get_input(player_t *player);  // Gets the input bitflags  
input_actions_t input_left_right_swap(bool is_facing_right, input_actions_t in);
void player_record_input(player_t *player, float delta_time);
bool player_check_combo(player_t *player, const input_sequence_t *seq);

// ================
//  IMPLEMENTATION
// ================

#ifdef PLAYER_IMPLEMENTATION

#include "macros.h"

// Swaps the left and right arows if facing left 
input_actions_t input_left_right_swap(bool is_facing_right, input_actions_t in)
{
    if (!is_facing_right)
    {
        if (!(in & INPUT_PRESSED_LEFT) != !(in & INPUT_PRESSED_RIGHT)) 
            in ^= (INPUT_PRESSED_LEFT | INPUT_PRESSED_RIGHT);
        
        if (!(in & INPUT_HOLDING_LEFT) != !(in & INPUT_HOLDING_RIGHT)) 
            in ^= (INPUT_HOLDING_LEFT | INPUT_HOLDING_RIGHT);
        
        if (!(in & INPUT_RELEASED_LEFT) != !(in & INPUT_RELEASED_RIGHT)) 
            in ^= (INPUT_RELEASED_LEFT | INPUT_RELEASED_RIGHT);
    }
    return in;
}

bool player_check_combo(player_t *player, const input_sequence_t *seq)
{
    if (seq->count <= 0) return false;

    int32_t history_len      = lenghtof(player->input_history);
    int32_t seq_step         = seq->count - 1;        // Start from the LAST step of sequence
    float newest_time        = player->input_timer;   // Newest input time
    const float COMBO_WINDOW = 0.65f;                 // Max seconds between each input in the sequence
    // The combo depends on fighters facing direction
    const bool do_swap = player->fighter.pysics.facing_right; 

    // walk history from newest [last] to oldest [0]
    for (int32_t i = (history_len - 1); i >= 0 && seq_step >= 0; i--)
    {
        const input_actions_t action = input_left_right_swap(do_swap, player->input_history[i]);
        const float timestamp        = player->input_timestamps[i];

        // skip frames with no relevant input
        if (action == INPUT_NONE) continue;

        // too much time passed between this input and the previous matched one
        if ((newest_time - timestamp) > COMBO_WINDOW)
            return false;

        // does this history entry match the current sequence step?
        if (action & seq->actions[seq_step])
        {
            newest_time = timestamp; // update window anchor
            seq_step--;              // matched — move to previous step
        }
    }

    // if seq_step < 0 we matched every step
    return (seq_step < 0);
}

input_actions_t player_get_input(player_t *player)
{
    return player->input_history[lenghtof(player->input_history) - 1]; 
}

void player_record_input(player_t *player, float delta_time)
{
    player->input_timer += delta_time;
    
    // Shift history: oldest <- newest  
    const uint32_t last = lenghtof(player->input_history) - 1;
    for_range_i(last)
    {
        player->input_history[i]    = player->input_history[i + 1];
        player->input_timestamps[i] = player->input_timestamps[i + 1];
    }
        
    input_actions_t input_action = INPUT_NONE;  // Bitfield for input_actions_t  
    for (uint32_t i = 0, flag = 1; i < BUTTON_COUNT; i++, flag <<= 1)
    {
        input_action |= (player->keybinds[i]->holding  * flag) << 0;
        input_action |= (player->keybinds[i]->pressed  * flag) << 8;
        input_action |= (player->keybinds[i]->released * flag) << 16;
    }

    player->input_history[last]    = input_action;
    player->input_timestamps[last] = player->input_timer;
}

bool init_players(input_t *input, player_t *p1, player_t *p2)
{
    // The cursor dosen't dissapear but it isnt tracked    
    SDL_ShowCursor(SDL_ENABLE);
    input->mouse.is_active = false; 
    
    // Keabord only
    const SDL_Scancode keys_p1[] = 
    {
        SDL_SCANCODE_W,
        SDL_SCANCODE_S,
        SDL_SCANCODE_D,
        SDL_SCANCODE_A,
        SDL_SCANCODE_F,
        SDL_SCANCODE_G,
        SDL_SCANCODE_H,
    };
    const SDL_Scancode keys_p2[] = 
    {
        SDL_SCANCODE_UP,
        SDL_SCANCODE_DOWN,
        SDL_SCANCODE_RIGHT,
        SDL_SCANCODE_LEFT,
        SDL_SCANCODE_I,
        SDL_SCANCODE_O,
        SDL_SCANCODE_P,
    };
    if (lenghtof(keys_p1) != (BUTTON_COUNT) || 
        lenghtof(keys_p2) != (BUTTON_COUNT))
    {
        return false;
    }

    for_range_i(BUTTON_COUNT)
    {
        p1->keybinds[i] = &input->keys[keys_p1[i]];
        p2->keybinds[i] = &input->keys[keys_p2[i]];
    }

    return true;
}

#endif /* PLAYER_IMPLEMENTATION */

#endif /* !_PLAYER_H */
