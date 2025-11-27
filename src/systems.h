#ifndef _SYSTEMS_H
#define _SYSTEMS_H

#include "game.h"

typedef void (*sys_func_t)(void *data);

#define SYS_MAX_LEN 32
#define INPUT_MAX_LEN 32

typedef enum {
    STATE_MENU,
    STATE_PLAY,
    STATE_PAUSE,
    STATE_COUNT
} state_tag_t;

typedef struct {
    sys_func_t funcs[SYS_MAX_LEN];
    void *data[SYS_MAX_LEN];
    uint8_t count;
} sys_table_t;

typedef struct {
    SDL_Scancode keys[INPUT_MAX_LEN];
    uint32_t frames[INPUT_MAX_LEN];
    uint8_t count;
} input_table_t;

typedef struct {
    sys_table_t sys;
    input_table_t input;
    void (*on_enter)(void);
    void (*on_exit)(void);
} state_t;

typedef struct {
    state_t states[STATE_COUNT];
    state_tag_t curr;
    state_tag_t next;
    bool should_switch;
} state_machine_t;

void sys_add(sys_table_t *sys_table, sys_func_t func, void *data);
void sys_remove(sys_table_t *sys_table, sys_func_t func);
void sys_run(sys_table_t *sys_table);

void queue_state(state_machine_t *machine, state_tag_t next_state);
void process_state(state_machine_t *machine);

void handle_events(game_t *game);
void handle_delta_time(struct game_time *time);

#endif /* _SYSTEMS_H */
