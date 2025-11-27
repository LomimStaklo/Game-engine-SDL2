#include "systems.h"

void sys_add(sys_table_t *sys_table, sys_func_t func, void *data) 
{
    if ( !(sys_table->count < SYS_MAX_LEN) ) return;
    
    sys_table->funcs[sys_table->count] = func;
    sys_table->data[sys_table->count++] = data;
}

void sys_remove(sys_table_t *sys_table, sys_func_t func)
{
    for (uint8_t i = 0; i < sys_table->count; i++)
    {
        if (sys_table->funcs[i] != func) continue; 
    
        for (uint8_t j = i; j < sys_table->count; j++)
            sys_table->funcs[j] = sys_table->funcs[j + 1];
        
        sys_table->count--;
        return;
    }
}

void sys_run(sys_table_t *sys_table)
{
    for (uint8_t i = 0; i < sys_table->count; i++)
        sys_table->funcs[i](sys_table->data[i]);
}

void queue_state(state_machine_t *machine, state_tag_t next_state) 
{
    machine->next = next_state;
    machine->should_switch = true;
}

void process_state(state_machine_t *machine)
{
    if ( !machine->should_switch ) return;
    else machine->should_switch = false;

    state_tag_t curr_state = machine->curr;
    state_tag_t next_state = machine->next;

    if (machine->states[curr_state].on_exit)
        machine->states[curr_state].on_exit();

    machine->curr = next_state;

    if (machine->states[next_state].on_enter)
        machine->states[next_state].on_enter();
}

void handle_delta_time(struct game_time *time)
{
    time->frame_elapsed = SDL_GetTicks() - time->frame_start;
    
    if (time->frame_elapsed < time->frame_rate)
        SDL_Delay(time->frame_rate - time->frame_elapsed);
    
    time->frame_start = SDL_GetTicks(); 
    time->frame_counter++;
}

void handle_events(game_t *game)
{
    switch (game->event.type) 
    {
        case SDL_QUIT: game->running = false; break;
        case SDL_WINDOWEVENT: 
            SDL_GetWindowSize(game->window, &game->position_win.w, &game->position_win.h); 
            break;
        case SDL_MOUSEMOTION:
            if ( !game->mouse.is_active ) break;
            game->mouse.button = SDL_GetMouseState(&game->mouse.x, &game->mouse.y);
            break;
        case SDL_KEYDOWN:
            if (game->event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) { game->running = false; }
            break;
        default: break;
    }
}

void handle_input(input_table_t *input) 
{
    
}