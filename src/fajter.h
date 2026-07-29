#ifndef _FAJTER_H
#define _FAJTER_H

// Header only file!
// For implementation you will need to define:
// #define FAJTER_IMPLEMENTATION

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include "assets.h"
#include "macros.h"

// -------------
//  DECLARATION
// -------------

struct fighter_t;
struct player_t;

// ---------------------------
// All states names 
#define FIGHTER_STATE_NAMES_XLIST \
X(IDLE) \
X(WALK_FORWARD) \
X(WALK_BACKWARD) \
X(AIRBORNE) \
X(KNOCKDOWN) \
X(RECOVERY) \
X(CROUCH) \
X(STAND_HITSTUN) \
X(CROUCH_HITSTUN) \
X(AIRBORNE_HITSTUN) \
X(AIRBORNE_ATK) \
X(STAND_BLOCK) \
X(CROUCH_BLOCK) \
X(STAND_LIGHT) \
X(STAND_MEDIUM) \
X(STAND_HEAVY) \
X(CROUCH_LIGHT) \
X(CROUCH_MEDIUM) \
X(CROUCH_HEAVY) \
X(POSE_VICTORY) \
X(POSE_DEFEAT) \
X(COMBO1) \
X(COMBO2) \
X(COMBO3) \
X(DASH_FORWARD) \
X(DASH_BACKWARD) \
X(SPECIAL1) \
X(SPECIAL2) \
// ---------------------------

/**
 * On framerate of 60fps:
 * 1 tick  = ~0.016 sec
 * 10 tick = ~0.16 sec
 * 15 tick = ~0.25 sec
 * 30 tick = ~0.50 sec
 * 60 tick = ~1.00 sec
 */
#define TICKS(t) (t)

typedef struct frame_t
{
    // Frame tile rect from the atlas
    SDL_Rect src;
    vec2i_t offset;
    
    int32_t ticks; // Amount of ticks frame will last (1 tick ~0.016 sec)

    // Collision
    uint8_t  count_hitboxs, count_hurtboxs;
    SDL_Rect hitboxs[2],    hurtboxs[2];
} frame_t;

typedef struct animation_def_t
{
    frame_t frames[10];
    int32_t frame_count;
    int32_t total_ticks;
    bool loop;
} animation_def_t;  

typedef struct animation_t
{
    const animation_def_t *animations; // Pointer to array of anims
    int32_t id;     // Index in animations array of current animation 
    int32_t texture;
    int32_t duration;
    uint16_t timer;  // current tick animation is on
    uint16_t frame; // current frame of animation is on
} animation_t;

typedef struct input_sequence_t 
{
    uint32_t actions[8]; // How may input actions does combo have if 0 it isnt a combo (input_actions)
    uint8_t count;       // The input sequence that needs to be done (caped at 8)
} input_sequence_t;

typedef enum attack_id_t
{
    ATK_ID_NONE = 0,
    ATK_ID_STAND_LIGHT,
    ATK_ID_STAND_MEDIUM,
    ATK_ID_STAND_HEAVY,
    ATK_ID_CROUCH_LIGHT,
    ATK_ID_CROUCH_MEDIUM,
    ATK_ID_CROUCH_HEAVY,
    ATK_ID_AIRBORNE_ATK,
    // Sequence move, the ones that need a comb input
    ATK_ID_COMBO1,
    ATK_ID_COMBO2,
    ATK_ID_COMBO3,
    ATK_ID_DASH_FORWARD,
    ATK_ID_DASH_BACKWARDS,
    ATK_ID_SPECIAL1, 
    ATK_ID_SPECIAL2,
    ATK_ID_COUNT
} attack_id_t;

typedef struct pysics_t
{
    vec2f_t position;
    vec2f_t velocity;
    bool facing_right;
    bool is_grounded;
} pysics_t;

typedef enum attack_trigger_t
{
    ATK_TRIGGER_ON_HIT = 0, // Default, func fires when attack connects
    ATK_TRIGGER_ON_COUNTER, // Fires only if enemy is in a hittable/attack state
    ATK_TRIGGER_IN_RANGE,   // Fires if enemy gets in attack->range 
} attack_trigger_t;

typedef enum attack_flags_t
{
    ATK_FLAG_NONE                 = 0,
    ATK_FLAG_ARMOR                = 1 << 0, // Absorbs a hit during startup
    ATK_FLAG_CANCEABLE            = 1 << 1, // Unblockable, uses grab detection
    ATK_FLAG_KNOCKDOWN            = 1 << 2, 
    ATK_FLAG_WALL_BOUNCE          = 1 << 3, 
    ATK_FLAG_CANT_BLOCK_STANDING  = 1 << 4, // Unblockable when standing
    ATK_FLAG_CANT_BLOCK_CROUCHING = 1 << 5, // Unblockable when crouching
} attack_flags_t;

typedef enum attack_kind_t
{
    ATK_KIND_SIMPLE = 0,
    ATK_KIND_GRAB,
    ATK_KIND_DASH,
    ATK_KIND_MULTIHIT,
    ATK_KIND_PROJECTILE,
} attack_kind_t;

typedef struct attack_simple_t
{
    int32_t damage;                    
    vec2f_t knockback;  // Push force on enemy    (pull if negative)
    vec2f_t recoil;     // Push force on attacker (forward launch if negative)
    int32_t stun_duration, hitstop;
    attack_flags_t flags;
    // If .func == NULL then stats above are applyed
    bool (*func)(struct fighter_t *atk, struct fighter_t *def, void *ctx);
} attack_simple_t;

typedef struct projectile_def_t
{
    animation_def_t anim;
    vec2f_t spawn_offset; // Spawns ant player position
        
    SDL_Rect hitbox;
    int32_t lifetime; // How many ticks before it disappears even if no hit
} projectile_def_t;

// TODO: add a passiv effect system 
typedef struct attack_t
{
    input_sequence_t sequence;
    attack_kind_t kind;
    attack_trigger_t triger;

    projectile_def_t projectile;
    attack_simple_t stats;     // Used for all attacks
    uint8_t multihit_interval; // Used for multihit attacks
    float range;           // Used for ATK_TRIGGER_IN_RANGE
    vec2f_t grab_offset;   // Used for teleporting enemy into a grab
    uint8_t grab_duration; // How long the grab lasts
    // TODO: this funcker doesnt want to work vec2f_t boost;
    
    uint8_t startup, active; // Used for attack window
} attack_t;

typedef enum fighter_state_id_t
{
#define X(name) STATE_ ## name,
    FIGHTER_STATE_NAMES_XLIST
    STATE_COUNT
#undef X
} fighter_state_id_t;

typedef struct state_def_t
{
    attack_id_t    attack;        // ATK_ID_NONE if not an attack state
    uint16_t       can_do;
} state_def_t;

typedef struct fighter_state_t
{
    int32_t id;     // Current state id
    int32_t timer;    // How long has state been running 
    int32_t duration; // if 0 then state stays forever  
} fighter_state_t;


typedef struct fighter_def_t
{
    animation_def_t animations[STATE_COUNT];
    attack_t attacks[ATK_ID_COUNT];

    const char *name;
    asset_name_t default_asset;
    
    int32_t base_hp;
    float jump_force;
    float walk_speed;
    
} fighter_def_t;

typedef struct fighter_t
{
    const fighter_def_t *def; // Fighter definition
    
    animation_t animation;
    
    pysics_t pysics;
    fighter_state_t state;
    //projectile_t projectiles[10];
    
    attack_id_t curr_attack_id;   // Current attack
    int32_t hit_landed_at;
    
    int32_t hp;
    uint32_t ragebait_meter;
    int32_t active_stun;   // Set by apply_hit, read in STATE_STAND_HITSTUN
} fighter_t;

void fighter_update_attack(fighter_t *atk, fighter_t *def, void *ctx);
void fighter_set_state(fighter_t *fighter, fighter_state_id_t next_state);
void fighter_update(struct player_t *player, fighter_t *fighter, float delta_time, int32_t floor_level);

SDL_Rect to_world_rect(fighter_t *fighter, SDL_Rect local);
float fighter_check_overlap(fighter_t *f1, fighter_t *f2);

// --------------------------------------
void pysics_update(pysics_t *object, float delta_time, int32_t timer, int32_t floor_level);
void pysics_facing_direction(pysics_t *obj1, pysics_t *obj2);
void animation_update(animation_t *anim);
void animation_change(animation_t *anim, int32_t next_anim, bool restart);
const frame_t *animation_get_frame(animation_t *anim);
SDL_Rect frame_rect_facing_position(const frame_t *frame, pysics_t *pysics);
// -------------------------------------- 

/* typedef struct fighter_state_defs_t_
{
    anims[STATE_COUNT];
    can_do[STATE_COUNT];
    attacks[ATK_ID_COUNT];
};

typedef struct state_t_
{
    
}; */
    
// ---- IMPLEMENTATION -----------------------
#ifdef FAJTER_IMPLEMENTATION

#include <stdio.h>
#include <math.h>
#include "player.h"
#include "match.h"

static void fighter_update_animation(fighter_t *fighter);

enum what_can_state_do_t {
    CAN_NOTHING     = 0,
    CAN_WALK        = 1 << 0,
    CAN_JUMP        = 1 << 1,
    CAN_ATK_LIGHT   = 1 << 2,
    CAN_ATK_MEDIUM  = 1 << 3,
    CAN_ATK_HEAVY   = 1 << 4,
    CAN_BLOCK       = 1 << 5,
    CAN_CROUCH      = 1 << 6,
    CAN_COMBO       = 1 << 7,
    CAN_DASH        = 1 << 8,
    
    CAN_ATK        = CAN_ATK_LIGHT | CAN_ATK_MEDIUM | CAN_ATK_HEAVY,
    CAN_EVERYTHING = CAN_WALK | CAN_JUMP | CAN_ATK | CAN_BLOCK | CAN_CROUCH | CAN_COMBO | CAN_DASH,
};

static const state_def_t state_defs[STATE_COUNT] = 
{
    [STATE_IDLE]          = {ATK_ID_NONE, CAN_EVERYTHING},
    [STATE_POSE_VICTORY]  = {ATK_ID_NONE, CAN_EVERYTHING},
    [STATE_POSE_DEFEAT]   = {ATK_ID_NONE, CAN_EVERYTHING},
    [STATE_WALK_FORWARD]  = {ATK_ID_NONE, CAN_EVERYTHING},
    [STATE_WALK_BACKWARD] = {ATK_ID_NONE, CAN_EVERYTHING},
    [STATE_AIRBORNE]      = {ATK_ID_NONE, CAN_ATK | CAN_COMBO},
    [STATE_AIRBORNE_ATK]  = {ATK_ID_AIRBORNE_ATK, CAN_COMBO},

    [STATE_CROUCH]        = {ATK_ID_NONE, CAN_ATK | CAN_BLOCK | CAN_CROUCH | CAN_COMBO | CAN_DASH},
    [STATE_STAND_BLOCK]   = {ATK_ID_NONE, CAN_ATK | CAN_BLOCK | CAN_CROUCH | CAN_COMBO | CAN_DASH},
    [STATE_CROUCH_BLOCK]  = {ATK_ID_NONE, CAN_ATK | CAN_BLOCK | CAN_CROUCH | CAN_COMBO | CAN_DASH},

    [STATE_STAND_LIGHT]   = {ATK_ID_STAND_LIGHT,   CAN_ATK_MEDIUM | CAN_ATK_HEAVY | CAN_COMBO},
    [STATE_STAND_MEDIUM]  = {ATK_ID_STAND_MEDIUM,  CAN_ATK_HEAVY | CAN_COMBO},
    [STATE_STAND_HEAVY]   = {ATK_ID_STAND_HEAVY,   CAN_COMBO},
    [STATE_CROUCH_LIGHT]  = {ATK_ID_CROUCH_LIGHT,  CAN_ATK_MEDIUM | CAN_ATK_HEAVY | CAN_COMBO},
    [STATE_CROUCH_MEDIUM] = {ATK_ID_CROUCH_MEDIUM, CAN_ATK_HEAVY | CAN_COMBO},
    [STATE_CROUCH_HEAVY]  = {ATK_ID_CROUCH_HEAVY,  CAN_COMBO},

    [STATE_DASH_FORWARD]  = {ATK_ID_DASH_FORWARD, CAN_JUMP | CAN_ATK | CAN_COMBO},     
    [STATE_DASH_BACKWARD] = {ATK_ID_DASH_BACKWARDS, CAN_JUMP | CAN_ATK | CAN_COMBO}, 
    
    [STATE_COMBO1]        = {ATK_ID_COMBO1, CAN_NOTHING},
    [STATE_COMBO2]        = {ATK_ID_COMBO2, CAN_NOTHING},
    [STATE_COMBO3]        = {ATK_ID_COMBO3, CAN_NOTHING},

    [STATE_SPECIAL1]      = {ATK_ID_SPECIAL1, CAN_NOTHING},
    [STATE_SPECIAL2]      = {ATK_ID_SPECIAL2, CAN_NOTHING},

    [STATE_STAND_HITSTUN]    = {ATK_ID_NONE, CAN_NOTHING},
    [STATE_CROUCH_HITSTUN]   = {ATK_ID_NONE, CAN_NOTHING},
    [STATE_AIRBORNE_HITSTUN] = {ATK_ID_NONE, CAN_NOTHING},
    [STATE_KNOCKDOWN]        = {ATK_ID_NONE, CAN_NOTHING},
    [STATE_RECOVERY]         = {ATK_ID_NONE, CAN_ATK | CAN_COMBO},
};

// ---- FORCE CLACULATION -----------------------------------------
#define FORCE_GRAVITY  900.0f // Units/sec^2 fall rate
#define FORCE_FRICTION 0.90f  // 0 == Instatnt stop, 1 == No stop

static float force_linear(float base, float rate, float time)
{
    return (rate * time) + base;
}

static inline bool is_fighter_airborn(fighter_t *fighter)
{
    return (
        fighter->state.id == STATE_AIRBORNE  || fighter->state.id == STATE_AIRBORNE_ATK || 
        fighter->state.id == STATE_KNOCKDOWN || fighter->state.id == STATE_AIRBORNE_HITSTUN
    ); 
}
static inline bool is_fighter_crouching(fighter_t *fighter)
{
    return (
        fighter->state.id == STATE_CROUCH       || fighter->state.id == STATE_CROUCH_BLOCK  || 
        fighter->state.id == STATE_CROUCH_LIGHT || fighter->state.id == STATE_CROUCH_MEDIUM ||
        fighter->state.id == STATE_CROUCH_HEAVY || fighter->state.id == STATE_CROUCH_HITSTUN
    );
}
static inline bool is_fighter_stuned(fighter_t *fighter)
{
    return (
        fighter->state.id == STATE_STAND_HITSTUN    || fighter->state.id == STATE_CROUCH_HITSTUN ||
        fighter->state.id == STATE_AIRBORNE_HITSTUN || fighter->state.id == STATE_KNOCKDOWN 
    );
}  
static inline bool is_fighter_immune(fighter_t *fighter)
{
    return (
        fighter->state.id == STATE_KNOCKDOWN || fighter->state.id == STATE_RECOVERY
    );
}

void pysics_update(pysics_t *object, float delta_time, int32_t timer, int32_t floor_level)
{
    // Actuall moving 
    vec2f_t velocity = vec2f_mul(object->velocity, vec2f(delta_time, delta_time));
    object->position = vec2f_add(object->position, velocity); // Applay velocity
    
    object->is_grounded = (object->position.y >= (float)floor_level) 
        ? true
        : false;
    
    if (!object->is_grounded) 
    {
        float grav = force_linear(FORCE_GRAVITY, 80.0f, (float)timer * delta_time);
        object->velocity.y += grav * delta_time; // Gravity 
    } else
    {
        object->position.y = (float)floor_level;
        object->velocity.y = 0.0f;
        object->velocity.x *= FORCE_FRICTION; // Slides to stop
    } 
}


void pysics_facing_direction(pysics_t *obj1, pysics_t *obj2)
{
    obj1->facing_right = (obj1->position.x < obj2->position.x)
        ? true 
        : false;
    obj2->facing_right = !obj1->facing_right;
}

static int32_t animation_def_total_ticks(const animation_def_t *anim)
{
    int32_t ticks = 0;
    for_range_i((unsigned)anim->frame_count)
        ticks += anim->frames[i].ticks;
    return ticks;
}

void animation_update(animation_t *anim)
{
    anim->timer++;
    
    const animation_def_t *curr = &anim->animations[anim->id];
    int32_t frame_timestamp = 0;
    
    for_range_i(anim->frame + 1)
        frame_timestamp += curr->frames[i].ticks;

    if (anim->timer >= frame_timestamp)
    {
        anim->frame++;
        
        if (anim->frame >= curr->frame_count)
        {    
            anim->frame =
                (curr->loop) ? 0 : (uint16_t)curr->frame_count - 1;
            anim->timer =
                (curr->loop) ? 0 : (uint16_t)animation_def_total_ticks(curr) - 1;
        }
    }
}

void animation_change(animation_t *anim, int32_t next_anim, bool restart)
{
    if (anim->id != next_anim) 
    {
        anim->id = next_anim;
        anim->timer = TICKS(0);
        
        anim->frame = (restart) ? 0 : (uint16_t)anim->animations[anim->id].frame_count - 1;
        anim->timer = (restart) ? TICKS(0) : (uint16_t)animation_def_total_ticks(&anim->animations[anim->id]) - 1;
    }
}
// Resets fighter->state_timer to 0
void fighter_set_state(fighter_t *fighter, fighter_state_id_t next_state)
{
    attack_id_t next_atk_id = state_defs[next_state].attack;

    // If next state is an attack 
    if (next_atk_id != ATK_ID_NONE)
    {
        // Canceable attack must be in recovery state
        if (fighter->curr_attack_id != ATK_ID_NONE)
        {
            const attack_t *attack = &fighter->def->attacks[fighter->curr_attack_id];

            bool canceable = (
                (fighter->curr_attack_id != next_atk_id) &&
                (attack->stats.flags & ATK_FLAG_CANCEABLE) && 
                (fighter->state.timer >= attack->startup)
            );
            if (!canceable) return;
        }
    }
    
    fighter->state.duration = (next_atk_id == ATK_ID_NONE) 
        ? 0 
        : fighter->def->animations[next_state].total_ticks;
    
    // TODO: Fix the game  
    if (next_state == STATE_RECOVERY) 
        fighter->state.duration = fighter->def->animations[next_state].total_ticks;

    fighter->state.id       = (int32_t)next_state;
    fighter->curr_attack_id = next_atk_id;
    fighter->hit_landed_at  = TICKS(-1);
    fighter->state.timer    = TICKS(0);
}   

void fighter_update(player_t *player, fighter_t *fighter, float delta_time, int32_t floor_level)
{
    fighter->state.timer++;
    
    // ---- PHYSICS -----------------------------------------------
    pysics_update(&fighter->pysics, delta_time, fighter->state.timer, floor_level);

    if (fighter->pysics.is_grounded) 
    {
        if (fighter->state.id == STATE_AIRBORNE || fighter->state.id == STATE_AIRBORNE_ATK) 
            fighter_set_state(fighter, STATE_IDLE);
    } else
    {    
        if (!is_fighter_airborn(fighter))            
            fighter_set_state(fighter, STATE_AIRBORNE);
    }

    const input_actions_t input = player_get_input(player);     // Current player input
    const input_actions_t swaped_input = input_left_right_swap(fighter->pysics.facing_right, input); 
    const uint16_t can_fighter_do = state_defs[fighter->state.id].can_do; // What state is alowed to do

    // ---- JUMP ------------------------------------------------------------------------
    if (can_fighter_do & CAN_JUMP && input & INPUT_PRESSED_UP)
    {
        // Jump force is positive stat number so it gets negated here
        fighter->pysics.velocity.y  = -fighter->def->jump_force; 
        fighter->pysics.is_grounded = false;
        fighter_set_state(fighter, STATE_AIRBORNE);
    }

    // ---- COMBO -----------------------------------------------------------------------
    // Starting form STATE_COMBO1 -> STATE_COUNT all attacks are combos
    // So there is no need to check the ones before 
    if (can_fighter_do & CAN_COMBO)
    {
        for (uint32_t state = STATE_COMBO1; state < STATE_COUNT; state++)
        {
            attack_id_t combo_id = state_defs[state].attack;
            assert(combo_id != ATK_ID_NONE && "You didnt acount for state_defs");

            if ((combo_id == ATK_ID_DASH_BACKWARDS || 
                combo_id == ATK_ID_DASH_FORWARD) &&
                (!(can_fighter_do & CAN_DASH))
            ) continue;

            if (player_check_combo(player, &fighter->def->attacks[combo_id].sequence))
            {
                fighter_set_state(fighter, state);
                goto state_machine;
            }
        }
    }

    // TODO: fix the mess with some kind on switch statement
    // Grounded states check (I know its a big if)
    if (fighter->pysics.is_grounded)
    {
        const bool crouching = is_fighter_crouching(fighter);
        
        if (can_fighter_do & CAN_CROUCH && input & INPUT_PRESSED_DOWN)
            fighter_set_state(fighter, STATE_CROUCH);

        if (can_fighter_do & CAN_WALK && input & INPUT_PRESSED_RIGHT) 
            fighter_set_state(fighter, STATE_WALK_FORWARD);
        
        if (can_fighter_do & CAN_WALK && input & INPUT_PRESSED_LEFT) 
            fighter_set_state(fighter, STATE_WALK_BACKWARD);

        // ---- ATTACKS ---------------------------------------------------------------------
        if (can_fighter_do & CAN_ATK_LIGHT && input & INPUT_PRESSED_LIGHT) 
        { 
            fighter_set_state(fighter, 
                (crouching) ? STATE_CROUCH_LIGHT : STATE_STAND_LIGHT); 
        }

        if (can_fighter_do & CAN_ATK_MEDIUM && input & INPUT_PRESSED_MEDIUM) 
        {
            fighter_set_state(fighter, 
                (crouching) ? STATE_CROUCH_MEDIUM : STATE_STAND_MEDIUM); 
        }

        if (can_fighter_do & CAN_ATK_HEAVY && input & INPUT_PRESSED_HEAVY) 
        {
            fighter_set_state(fighter, 
                (crouching) ? STATE_CROUCH_HEAVY : STATE_STAND_HEAVY); 
        } 
        
        // ---- BLOCK -----------------------------------------------------------------------
        if (can_fighter_do & CAN_BLOCK && ((swaped_input & INPUT_HOLDING_BLOCK) == INPUT_HOLDING_BLOCK))
        {
            fighter_set_state(
                fighter,
                (crouching) ? STATE_CROUCH_BLOCK : STATE_STAND_BLOCK
            );
        }
    } 
    // Airborn input
    else 
    {
        if (can_fighter_do & CAN_ATK &&
            input & (INPUT_PRESSED_LIGHT | INPUT_PRESSED_MEDIUM | INPUT_PRESSED_HEAVY))
        {
            fighter_set_state(fighter, STATE_AIRBORNE_ATK);
        }
    }
    
    // ---- STATE MACHINE -------------------------------------------------------------------
state_machine:;
    fighter_state_id_t state = (fighter_state_id_t)fighter->state.id; 
    bool is_stun_active   = fighter->state.timer < fighter->active_stun;
    bool is_state_active  = fighter->state.timer < fighter->state.duration; 
    bool is_airborn       = is_fighter_airborn(fighter);
    bool is_crouching     = is_fighter_crouching(fighter);
    switch (state)
    {
        // ---- IDLE ------------------------------------------------------------------------
        case STATE_POSE_VICTORY: break;
        case STATE_POSE_DEFEAT: break;
        case STATE_IDLE: 
        {
            if (input & INPUT_HOLDING_LEFT) 
                fighter_set_state(fighter, STATE_WALK_FORWARD);

            if (input & INPUT_HOLDING_RIGHT) 
                fighter_set_state(fighter, STATE_WALK_BACKWARD);

            if (input & INPUT_HOLDING_DOWN)
                fighter_set_state(fighter, STATE_CROUCH);
 
            break;
        }
        // ---- WALK ------------------------------------------------------------------------
        case STATE_WALK_FORWARD:
        case STATE_WALK_BACKWARD:
        {
            if (input & INPUT_HOLDING_LEFT) 
                fighter->pysics.velocity.x = -fighter->def->walk_speed;
            
            else if (input & INPUT_HOLDING_RIGHT) 
                fighter->pysics.velocity.x = fighter->def->walk_speed;
            
            else
                fighter_set_state(fighter, STATE_IDLE);
            break;
        }
        // ---- DASH ------------------------------------------------------------------------
        case STATE_DASH_FORWARD:
        case STATE_DASH_BACKWARD:
        {
            if (fighter->state.timer <= TICKS(1))
            {
                float dir = (fighter->state.id == STATE_DASH_FORWARD) == fighter->pysics.facing_right
                    ? 1.0f 
                    : -1.0f;
                fighter->pysics.velocity.x = dir * (fighter->def->walk_speed * 2.0f);
            }

            if (fabsf(fighter->pysics.velocity.x) < 30.0f && 
                (fighter->hit_landed_at == -1 || (!is_state_active)))
            {
                fighter_set_state(fighter, (state == STATE_DASH_FORWARD)
                    ? STATE_WALK_FORWARD 
                    : STATE_WALK_BACKWARD
                );
            }
            break;
        }
        // ---- JUMP ------------------------------------------------------------------------
        case STATE_AIRBORNE: 
        {
            // Air steering
            if (input & INPUT_HOLDING_LEFT)       fighter->pysics.velocity.x = -fighter->def->walk_speed;
            else if (input & INPUT_HOLDING_RIGHT) fighter->pysics.velocity.x = fighter->def->walk_speed;

            break;
        }
        // ---- CROUCH ----------------------------------------------------------------------
        case STATE_CROUCH:
        {
            // Stay crouching while holding input DOWN
            if (!(input & INPUT_HOLDING_DOWN)) {
                fighter_set_state(fighter, STATE_IDLE);
            }
            break;
        }
        // ---- BLOCK -----------------------------------------------------------------------
        case STATE_STAND_BLOCK: 
        case STATE_CROUCH_BLOCK: 
        {
            fighter->pysics.velocity.x = 0.0f;

            if (input & INPUT_HOLDING_DOWN) fighter_set_state(fighter, STATE_CROUCH_BLOCK);
            else                            fighter_set_state(fighter, STATE_STAND_BLOCK);
            
            if (!((swaped_input & INPUT_HOLDING_BLOCK) == INPUT_HOLDING_BLOCK))
            {
                fighter_set_state(fighter, (state == STATE_STAND_BLOCK) 
                    ? STATE_IDLE 
                    : STATE_CROUCH
                );
            }
            break;
        }
        // ---- HITSTUNS --------------------------------------------------------------------
        case STATE_STAND_HITSTUN:
        case STATE_CROUCH_HITSTUN:
        case STATE_AIRBORNE_HITSTUN:
        {
            if (!is_stun_active)
            {
                fighter->active_stun = TICKS(0);

                if (is_crouching)    fighter_set_state(fighter, STATE_CROUCH);
                else if (is_airborn) fighter_set_state(fighter, STATE_AIRBORNE);
                else                 fighter_set_state(fighter, STATE_IDLE);
            }
            break;
        }
        // ---- KNOCKDOWN -------------------------------------------------------------------
        case STATE_KNOCKDOWN:
        {
            // after half of the 'active_stun' time pased it gets to a STATE_RECOVERY
            if (!is_stun_active)
            {
                fighter->active_stun = TICKS(0);
                fighter_set_state(fighter, STATE_RECOVERY);
            }
            break;
        } 
        // ---- ATTACKS ---------------------------------------------------------------------
        case STATE_AIRBORNE_ATK:     
        case STATE_CROUCH_LIGHT:
        case STATE_CROUCH_MEDIUM:
        case STATE_CROUCH_HEAVY:
        case STATE_STAND_LIGHT:  
        case STATE_STAND_MEDIUM: 
        case STATE_STAND_HEAVY:  
        case STATE_COMBO1:
        case STATE_COMBO2:
        case STATE_COMBO3:
        case STATE_SPECIAL1:
        case STATE_SPECIAL2:
        case STATE_RECOVERY:
        {
            if (!is_state_active)
            {
                if (is_crouching)    fighter_set_state(fighter, STATE_CROUCH);
                else if (is_airborn) fighter_set_state(fighter, STATE_AIRBORNE);
                else                 fighter_set_state(fighter, STATE_IDLE);
            }
            break;
        }

        case STATE_COUNT: break; // Ignore
    }
    // Animation update
    fighter_update_animation(fighter);
}

static void fighter_update_animation(fighter_t *fighter)
{
    animation_update(&fighter->animation);
        
    // ---- ANIM STATE ------------------------------------------------------
    // NOTE: The function is comparing the animation id with current state id
    // to see if it needs to switch and if the animation should be restarted 
    const fighter_state_id_t curr_anim = (fighter_state_id_t)fighter->animation.id;
    fighter_state_id_t       next_anim = (fighter_state_id_t)fighter->state.id;  
    bool restart = true; // restart animation 

    if (curr_anim == STATE_AIRBORNE_ATK || curr_anim == STATE_AIRBORNE_HITSTUN) 
        restart = false;
    if ((next_anim == STATE_CROUCH) && (
        curr_anim == STATE_CROUCH_LIGHT   || 
        curr_anim == STATE_CROUCH_MEDIUM  || 
        curr_anim == STATE_CROUCH_HEAVY   ||
        curr_anim == STATE_CROUCH_HITSTUN ||
        curr_anim == STATE_CROUCH_BLOCK)
    ) restart = false;

    if (next_anim == STATE_WALK_FORWARD || next_anim == STATE_WALK_BACKWARD) 
    {
        next_anim = (fighter->pysics.velocity.x > 0) == fighter->pysics.facing_right
            ? STATE_WALK_FORWARD 
            : STATE_WALK_BACKWARD;
    }

    // Animation switching 
    animation_change(&fighter->animation, (int32_t)next_anim, restart);
}

const frame_t *animation_get_frame(animation_t *anim)
{
    return &anim->animations[anim->id].frames[anim->frame];
}

/* bool frame_check_hit(frame_t *fr_atk, frame_t *fr_def)
{
    for_range_i(fr_atk->count_hitboxs)
    {
        const SDL_Rect hit = to_world_rect(atk, fr_atk->hitboxs[i]);

        for_range_j(fr_def->count_hurtboxs)
        {
            const SDL_Rect hurt = to_world_rect(def, fr_def->hurtboxs[j]);

            if (SDL_HasIntersection(&hit, &hurt))
                return true;
        }
    }
} */

SDL_Rect frame_rect_facing_position(const frame_t *frame, pysics_t *pysics)
{
    SDL_Rect rect;
    if (pysics->facing_right)
        rect.x = (int32_t)pysics->position.x - frame->offset.x;
    else
        rect.x = (int32_t)pysics->position.x - (frame->src.w - frame->offset.x);

    rect.y = (int32_t)pysics->position.y - frame->offset.y;
    rect.w = frame->src.w; 
    rect.h = frame->src.h;

    return rect;
}

SDL_Rect to_world_rect(fighter_t *fighter, SDL_Rect local)
{
    const frame_t *frame = animation_get_frame(&fighter->animation);

    // top-left corner of the sprite in world space
    float sprite_left, sprite_top;

    if (fighter->pysics.facing_right)
        sprite_left = fighter->pysics.position.x - (float)frame->offset.x;
    else
        // flipped: offset_x measured from right edge instead
        sprite_left = fighter->pysics.position.x - (float)(frame->src.w - frame->offset.x);

    sprite_top = fighter->pysics.position.y - (float)frame->offset.y;

    SDL_Rect world;

    if (fighter->pysics.facing_right)
    {
        world.x = (int32_t)sprite_left + local.x;
    }
    else
    {
        // mirror the box horizontally within the sprite
        // local.x is from left edge of sprite, when flipped it becomes from right edge
        world.x = (int32_t)sprite_left + (frame->src.w - local.x - local.w);
    }

    world.y = (int32_t)sprite_top + local.y;
    world.w = local.w;
    world.h = local.h;

    return world;
}

static bool fighter_check_hit(fighter_t *atk, fighter_t *def)
{
    const frame_t *col_atk = animation_get_frame(&atk->animation);
    const frame_t *col_def = animation_get_frame(&def->animation);

    for_range_i(col_atk->count_hitboxs)
    {
        const SDL_Rect hit = to_world_rect(atk, col_atk->hitboxs[i]);

        for_range_j(col_def->count_hurtboxs)
        {
            const SDL_Rect hurt = to_world_rect(def, col_def->hurtboxs[j]);

            if (SDL_HasIntersection(&hit, &hurt))
                return true;
        }
    }
    return false;
}

/** \returns overlap amout on x axis, or zero */
float fighter_check_overlap(fighter_t *f1, fighter_t *f2)
{
    const frame_t *f1_col = animation_get_frame(&f1->animation);
    const frame_t *f2_col = animation_get_frame(&f2->animation);

    for_range_i(f1_col->count_hurtboxs)
    {
        const SDL_Rect f1_hurt = to_world_rect(f1, f1_col->hurtboxs[i]);

        for_range_j(f2_col->count_hurtboxs)
        {
            const SDL_Rect f2_hurt = to_world_rect(f2, f2_col->hurtboxs[j]);
            
            if (SDL_HasIntersection(&f1_hurt, &f2_hurt)) 
            {
                float f1_right = (float)(f1_hurt.x + f1_hurt.w);
                float f2_right = (float)(f2_hurt.x + f2_hurt.w);
                
                float overlap = (f1_right < f2_right)
                    ? f1_right - (float)f2_hurt.x
                    : f2_right - (float)f1_hurt.x;
                
                return overlap;
            } else
                continue;
        }
    }
    return 0.0f;
}

// TODO: Make this used for fighters and projectile because 
// this func is going set .hit_landed_at = timer and  
// to apply recoil to owner of a projectile
static void attack_apply_simple(const attack_t *simple, fighter_t *atk, fighter_t *def, void *ctx)
{
    const attack_simple_t *attack = &simple->stats;
    
    if (attack->func)
    {
        attack->func(atk, def, ctx); 
        return;
    }
    // Attack context
    bool crouching = is_fighter_crouching(def); 
    bool airborn = is_fighter_airborn(def); 
    bool blocked = ( 
        (def->state.id == STATE_STAND_BLOCK  && !(attack->flags & ATK_FLAG_CANT_BLOCK_STANDING)) ||
        (def->state.id == STATE_CROUCH_BLOCK && !(attack->flags & ATK_FLAG_CANT_BLOCK_CROUCHING))
    );

    int32_t damage = (blocked) // 90% of damage is blocked
        ? (int32_t)((float)attack->damage * 0.10f)
        : attack->damage;
    
    float dir = atk->pysics.facing_right ? 1.0f : -1.0f;
    
    vec2f_t knockback = 
        vec2f_mul((blocked) // 90% of knockback is blocked
            ? vec2f_mul(attack->knockback, vec2f(0.10f, 0.10f))
            : attack->knockback,
            vec2f(dir, 1.0f)
        );

    vec2f_t recoil = vec2f_mul(attack->recoil, vec2f(-dir, 1.0f));
    uint32_t atk_ragebait = (!blocked) ? (uint32_t)((float)attack->damage * 0.50f) : 0;
    uint32_t def_ragebait = (!blocked) ? (uint32_t)((float)attack->damage * 0.75f) : (uint32_t)attack->damage;

    // Applying the attack
    def->hp             -= damage;
    def->pysics.velocity = knockback;
    def->ragebait_meter += def_ragebait;

    atk->hit_landed_at   = atk->state.timer;
    atk->pysics.velocity = recoil;
    atk->ragebait_meter += atk_ragebait; 

    if (!blocked)
    {
        def->active_stun = attack->stun_duration;
        if (ctx != NULL)
        {
            match_t *match = (match_t *)ctx;
            match->hitstop = attack->hitstop; 
        }

        if (attack->flags & ATK_FLAG_KNOCKDOWN) fighter_set_state(def, STATE_KNOCKDOWN);
        else
        {
            if (crouching)    fighter_set_state(def, STATE_CROUCH_HITSTUN);
            else if (airborn) fighter_set_state(def, STATE_AIRBORNE_HITSTUN);
            else              fighter_set_state(def, STATE_STAND_HITSTUN);
        }
    }
}

// ---- FIGHTER ATTACK ------------------------------------------------------ 
void fighter_update_attack(fighter_t *atk, fighter_t *def, void *ctx)
{
    if (atk->curr_attack_id == ATK_ID_NONE) return;

    const attack_t *attack = &atk->def->attacks[atk->curr_attack_id];
    int32_t tick = atk->state.timer;

    // ---- ACTIVE WINDOW ---------------------------------------------------
    bool in_active_window = (
        tick >= (int32_t)attack->startup &&
        tick <  (int32_t)(attack->startup + attack->active)
    );
    if (!in_active_window) return;

    // ---- TRIGGER ---------------------------------------------------------
    bool trigger = false;
    switch (attack->triger)
    {
        case ATK_TRIGGER_ON_HIT:
            trigger = fighter_check_hit(atk, def);
            break;

        case ATK_TRIGGER_ON_COUNTER:
            trigger = fighter_check_hit(def, atk);
            break;

        case ATK_TRIGGER_IN_RANGE:
        {
            vec2f_t dist = vec2f_sub(atk->pysics.position, def->pysics.position);
            dist.x = fabsf(dist.x); 
            dist.y = fabsf(dist.y);
            trigger = (attack->range >= dist.x || attack->range >= dist.y);
            break;
        }
    }

    switch (attack->kind)
    {
        case ATK_KIND_SIMPLE:
        {
            // Attack applyed only once
            if (atk->hit_landed_at == TICKS(-1) && trigger)
                attack_apply_simple(attack, atk, def, ctx);
            break;
        }
        case ATK_KIND_MULTIHIT:
        {
            bool is_last_hit = (tick == (attack->startup + attack->active - 1));
            
            if (is_last_hit && trigger)
                attack_apply_simple(attack, atk ,def, ctx);

            else if ((tick % attack->multihit_interval == 0) && trigger)
                attack_apply_simple(attack, atk ,def, ctx);
            break;
        }
        case ATK_KIND_GRAB:
        {
            // TODO: Decide if enemy could be grabbed while attacking 
            if (!is_fighter_stuned(def) && !is_fighter_immune(def) && trigger)
            {
                vec2f_t offset = (atk->pysics.facing_right) 
                    ? attack->grab_offset
                    : vec2f(-1.0f * attack->grab_offset.x, attack->grab_offset.y);
                
                // Teleports defender to grab position
                def->pysics.position = vec2f_add(atk->pysics.position, offset);

                fighter_set_state(def, STATE_AIRBORNE_HITSTUN);
                def->active_stun = attack->grab_duration;
            
                if (atk->state.timer >= attack->grab_duration && atk->hit_landed_at == TICKS(-1))
                    attack_apply_simple(attack, atk, def, ctx);
            }
            break;
        }
        case ATK_KIND_DASH:
        {
            if (atk->hit_landed_at == TICKS(-1) && trigger)
                attack_apply_simple(attack, atk, def, ctx);
            break;
        }
    }
}


#endif /* FAJTER_IMPLEMENTATION */

#endif /* !_FAJTER_H */
