#ifndef _FAJTER_H
#define _FAJTER_H

// Header only file!
// For implementation you will need to define:
// #define FAJTER_IMPLEMENTATION

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include "assets.h"

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
X(DASH_FORWARD) \
X(DASH_BACKWARD) \
X(POSE_VICTORY) \
X(COMBO1) \
X(COMBO2) \
X(COMBO3) \
X(SPECIAL1) \
X(SPECIAL2) \
// ---------------------------

// Animation type is prefixed with ANIM_
typedef enum animation_id_t 
{
#define X(name) ANIM_ ## name,
    FIGHTER_STATE_NAMES_XLIST
    ANIM_COUNT
#undef X
} animation_id_t;

typedef struct anim_frame_t
{
    // Frame tile rect from the atlas
    SDL_Rect src;
    int32_t  offset_x, offset_y;
    // Collision
    uint8_t  count_hitboxs, count_hurtboxs;
    SDL_Rect hitboxs[4],    hurtboxs[4];
} anim_frame_t;

typedef struct animation_t
{
    float frame_duration;
    int32_t frame_count;
    uint8_t startup_frames, active_frames;
    bool loop;
    anim_frame_t frames[8];
} animation_t;  

typedef struct fighter_visuals_t
{
    asset_name_t default_asset;
    animation_t animations[ANIM_COUNT];
} fighter_visuals_t;

typedef struct input_sequence_t 
{
    uint32_t actions[8];  // How may input actions does combo have if 0 it isnt a combo (input_actions)
    uint8_t count;       // The input sequence that needs to be done (caped at 8)
} input_sequence_t;

typedef enum attack_trigger_t
{
    ATK_TRIGGER_ON_HIT     = 0, // Default, func fires when attack connects
    ATK_TRIGGER_ON_COUNTER = 1, // Fires only if enemy is in a hittable/attack state
    ATK_TRIGGER_ON_WHIFF   = 2, // Fires when attack ends without hitting
    ATK_TRIGGER_ON_BLOCK   = 3, // Fires when enemy blocks it
} attack_trigger_t;

typedef enum attack_flags_t
{
    ATK_FLAG_NONE                 = 0,
    ATK_FLAG_ARMOR                = 1 << 0, // Absorbs a hit during startup
    ATK_FLAG_GRAB                 = 1 << 1, // Unblockable, uses grab detection
    ATK_FLAG_PROJECTILE           = 1 << 2, // Spawns a separate hitbox entity
    ATK_FLAG_KNOCKDOWN            = 1 << 3, 
    ATK_FLAG_WALL_BOUNCE          = 1 << 4, 
    ATK_FLAG_CANT_BLOCK_STANDING  = 1 << 5, // Unblockable when standing
    ATK_FLAG_CANT_BLOCK_CROUCHING = 1 << 6, // Unblockable when crouching
    ATK_FLAG_CANCEABLE            = 1 << 7, // Move can be canceled
    ATK_FLAG_MULTIHIT             = 1 << 8, // apply full knockback/stun only on last active frame
} attack_flags_t;

// TODO: add a passiv effect system 
typedef struct attack_t
{
    int32_t damage;                    
    float knockback_x, knockback_y;  // Push force on enemy    (pull if negative)
    float recoil_x, recoil_y;        // Push force on attacker (forward launch if negative)
    float stun_duration;             // Duration of an attack and its stun effect on enemy

    attack_trigger_t triger;
    attack_flags_t flags;
    
    // If func is NULL than it isnt executed
    void (*func)(struct fighter_t *atk, struct fighter_t *def, void *ctx);
    input_sequence_t sequence;
} attack_t;

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
    ATK_ID_SPECIAL1, 
    ATK_ID_SPECIAL2,
    ATK_ID_COUNT
} attack_id_t;

typedef enum fighter_state_t
{
#define X(name) STATE_ ## name,
    FIGHTER_STATE_NAMES_XLIST
    STATE_COUNT
#undef X
} fighter_state_t;

typedef struct state_def_t
{
    animation_id_t  anim;          // What animation this state plays
    attack_id_t     attack;        // ATK_ID_NONE if not an attack state
    uint16_t        can_do;
} state_def_t;

typedef struct fighter_t
{
    const char *name;

    int32_t texture;
    const fighter_visuals_t *visuals; // Sprite atlas and collisions
    const animation_t *animation;       // Current animation
    animation_id_t     animation_id;    // Same animation 
    float              animation_timer; // How long is the animation playing
    int32_t            animation_frame; // At what frame is the animation currently on
    
    fighter_state_t state; // Current state
    float state_timer;     // How long has state been running 
    float jump_force;
    float walk_speed;    
    
    // ---- STATS AND VARS ------------------------------------------------------------
    int32_t hp;
    uint32_t ragebait_meter;

    float position_x, position_y;
    float velocity_x, velocity_y;
    
    float active_stun_duration;   // Set by apply_hit, read in STATE_STAND_HITSTUN
    float active_atk_duration;    // Set when entering an attack state

    bool facing_right;
    bool is_grounded;
    int32_t last_hit_frame;

    // Attack stats
    attack_id_t curr_attack_id;   // Current attack
    attack_t attacks[ATK_ID_COUNT];
} fighter_t;


void fighter_check_attack(fighter_t *atk, fighter_t *def, void *ctx);
void fighter_set_state(fighter_t *fighter, fighter_state_t next_state);
void fighter_update(struct player_t *player, fighter_t *fighter, float delta_time);

const anim_frame_t *fighter_get_frame_data(fighter_t *fighter);
SDL_Rect to_world_rect(fighter_t *fighter, SDL_Rect local);
float fighter_check_overlap(fighter_t *f1, fighter_t *f2);

// ----------------
//  IMPLEMENTATION
// ----------------

#ifdef FAJTER_IMPLEMENTATION

#include "player.h"
#include <stdio.h>

static void fighter_update_animation(fighter_t *fighter, float delta_time);

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

static state_def_t state_defs[STATE_COUNT] = 
{
    [STATE_IDLE]          = {ANIM_IDLE,          ATK_ID_NONE, CAN_EVERYTHING},
    [STATE_POSE_VICTORY]  = {ANIM_POSE_VICTORY,  ATK_ID_NONE, CAN_EVERYTHING},
    [STATE_WALK_FORWARD]  = {ANIM_WALK_FORWARD,  ATK_ID_NONE, CAN_EVERYTHING},
    [STATE_WALK_BACKWARD] = {ANIM_WALK_BACKWARD, ATK_ID_NONE, CAN_EVERYTHING},
    [STATE_AIRBORNE]      = {ANIM_AIRBORNE,      ATK_ID_NONE, CAN_ATK | CAN_COMBO},
    [STATE_AIRBORNE_ATK]  = {ANIM_AIRBORNE_ATK,  ATK_ID_AIRBORNE_ATK, CAN_COMBO},

    [STATE_CROUCH]        = {ANIM_CROUCH,       ATK_ID_NONE, CAN_ATK | CAN_BLOCK | CAN_CROUCH | CAN_COMBO | CAN_DASH},
    [STATE_STAND_BLOCK]   = {ANIM_STAND_BLOCK,  ATK_ID_NONE, CAN_ATK | CAN_BLOCK | CAN_CROUCH | CAN_COMBO | CAN_DASH},
    [STATE_CROUCH_BLOCK]  = {ANIM_CROUCH_BLOCK, ATK_ID_NONE, CAN_ATK | CAN_BLOCK | CAN_CROUCH | CAN_COMBO | CAN_DASH},

    [STATE_STAND_LIGHT]   = {ANIM_STAND_LIGHT,   ATK_ID_STAND_LIGHT,   CAN_ATK_MEDIUM | CAN_ATK_HEAVY | CAN_COMBO},
    [STATE_STAND_MEDIUM]  = {ANIM_STAND_MEDIUM,  ATK_ID_STAND_MEDIUM,  CAN_ATK_HEAVY | CAN_COMBO},
    [STATE_STAND_HEAVY]   = {ANIM_STAND_HEAVY,   ATK_ID_STAND_HEAVY,   CAN_COMBO},
    [STATE_CROUCH_LIGHT]  = {ANIM_CROUCH_LIGHT,  ATK_ID_CROUCH_LIGHT,  CAN_ATK_MEDIUM | CAN_ATK_HEAVY | CAN_COMBO},
    [STATE_CROUCH_MEDIUM] = {ANIM_CROUCH_MEDIUM, ATK_ID_CROUCH_MEDIUM, CAN_ATK_HEAVY | CAN_COMBO},
    [STATE_CROUCH_HEAVY]  = {ANIM_CROUCH_HEAVY,  ATK_ID_CROUCH_HEAVY,  CAN_COMBO},

    [STATE_DASH_FORWARD]  = {ANIM_DASH_FORWARD,  ATK_ID_NONE, CAN_JUMP | CAN_ATK | CAN_COMBO},     
    [STATE_DASH_BACKWARD] = {ANIM_DASH_BACKWARD, ATK_ID_NONE, CAN_JUMP | CAN_ATK | CAN_COMBO}, 
    
    [STATE_COMBO1]        = {ANIM_COMBO1, ATK_ID_COMBO1, CAN_NOTHING},
    [STATE_COMBO2]        = {ANIM_COMBO2, ATK_ID_COMBO2, CAN_NOTHING},
    [STATE_COMBO3]        = {ANIM_COMBO3, ATK_ID_COMBO3, CAN_NOTHING},

    [STATE_SPECIAL1]      = {ANIM_SPECIAL1, ATK_ID_SPECIAL1, CAN_NOTHING},
    [STATE_SPECIAL2]      = {ANIM_SPECIAL2, ATK_ID_SPECIAL2, CAN_NOTHING},

    [STATE_STAND_HITSTUN]    = {ANIM_STAND_HITSTUN, ATK_ID_NONE, CAN_NOTHING},
    [STATE_CROUCH_HITSTUN]   = {ANIM_CROUCH_HITSTUN, ATK_ID_NONE, CAN_NOTHING},
    [STATE_AIRBORNE_HITSTUN] = {ANIM_AIRBORNE_HITSTUN, ATK_ID_NONE, CAN_NOTHING},
    [STATE_KNOCKDOWN]        = {ANIM_KNOCKDOWN, ATK_ID_NONE, CAN_NOTHING},
    [STATE_RECOVERY]         = {ANIM_RECOVERY, ATK_ID_NONE, CAN_ATK | CAN_COMBO},
};

// ---- FORCE CLACULATION -----------------------------------------
#define FORCE_GRAVITY  900.0f // Units/sec^2 fall rate
#define FORCE_FRICTION 0.85f  // 0 == Instatnt stop, 1 == No stop
#define FLOOR_Y_LEVEL  (SCREEN_HEIGHT - 30)  

static float force_linear(float base, float rate, float time)
{
    return base + (rate * time);
}

// Resets fighter->state_timer to 0
void fighter_set_state(fighter_t *fighter, fighter_state_t next_state)
{
    attack_id_t curr_atk_id = fighter->curr_attack_id;
    bool canceable = 
        ((fighter->last_hit_frame == -1) && 
        (curr_atk_id == ATK_ID_NONE ||
        (fighter->attacks[curr_atk_id].flags & ATK_FLAG_CANCEABLE)))
            ? true
            : false;

    animation_id_t anim_id = state_defs[next_state].anim;
    const animation_t *anim = &fighter->visuals->animations[anim_id];
 
    attack_id_t next_atk_id = state_defs[next_state].attack;
    if (next_atk_id != ATK_ID_NONE && canceable)
    {
        fighter->curr_attack_id      = next_atk_id;
        fighter->active_atk_duration = anim->frame_duration * (float)anim->frame_count;
        fighter->last_hit_frame      = -1;

        fighter->state = next_state;
        fighter->state_timer = 0.0f;
    }
    else if (next_atk_id == ATK_ID_NONE)
    {
        fighter->last_hit_frame = -1;
        fighter->curr_attack_id = ATK_ID_NONE;
        fighter->active_atk_duration = 0.0f;

        fighter->state = next_state;
        fighter->state_timer = 0.0f;
    }
}   

void fighter_update(player_t *player, fighter_t *fighter, float delta_time)
{
    fighter->state_timer += delta_time;
    
    // ---- PHYSICS -----------------------------------------------
    if (!fighter->is_grounded) 
    {
        if (!(fighter->state == STATE_AIRBORNE || fighter->state == STATE_AIRBORNE_ATK ||
            fighter->state == STATE_AIRBORNE_HITSTUN || fighter->state == STATE_KNOCKDOWN))
            fighter_set_state(fighter, STATE_AIRBORNE);

        const float grav = force_linear(FORCE_GRAVITY, 80.0f, fighter->state_timer);
        fighter->velocity_y += grav * delta_time; // Gravity 
    } else
        fighter->velocity_x *= FORCE_FRICTION; // Slides to stop

    fighter->position_x += fighter->velocity_x * delta_time;
    fighter->position_y += fighter->velocity_y * delta_time;

    if (fighter->position_y >= (float)FLOOR_Y_LEVEL) 
    {
        fighter->position_y = (float)FLOOR_Y_LEVEL;
        fighter->velocity_y = 0.0f;
        fighter->is_grounded = true;
        if (fighter->state == STATE_AIRBORNE || fighter->state == STATE_AIRBORNE_ATK) 
            fighter_set_state(fighter, STATE_IDLE);
    }
    else
        fighter->is_grounded = false;

    const input_actions_t input = player_get_input(player);     // Current player input
    const input_actions_t swaped_input = input_left_right_swap(fighter->facing_right, input); 
    const uint16_t can_fighter_do = state_defs[fighter->state].can_do; // What state is alowed to do

    // TODO: fix the mess with some kind on switch statement
    // Grounded states check (I know its a big if)
    if (fighter->is_grounded)
    {
        const bool crouching = 
            (fighter->state == STATE_CROUCH      || fighter->state == STATE_CROUCH_BLOCK  || 
            fighter->state == STATE_CROUCH_LIGHT || fighter->state == STATE_CROUCH_MEDIUM ||
            fighter->state == STATE_CROUCH_HEAVY);
        
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
        // ---- JUMP ------------------------------------------------------------------------
        if (can_fighter_do & CAN_JUMP && input & INPUT_PRESSED_UP)
        {
            fighter->velocity_y  = -fighter->jump_force; // Jump force is positive stat number so it gets negated here
            fighter->is_grounded = false;
            fighter_set_state(fighter, STATE_AIRBORNE);
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
    // ---- DASH ------------------------------------------------------------------------
    if (can_fighter_do & CAN_DASH)
    {
        input_sequence_t dash_f = {{INPUT_PRESSED_RIGHT, INPUT_PRESSED_RIGHT}, 2};
        input_sequence_t dash_b = {{INPUT_PRESSED_LEFT, INPUT_PRESSED_LEFT}, 2};
        
        if (player_check_combo(player, &dash_f)) fighter_set_state(fighter, STATE_DASH_FORWARD);
        if (player_check_combo(player, &dash_b)) fighter_set_state(fighter, STATE_DASH_BACKWARD);
    }

    // ---- COMBO -----------------------------------------------------------------------
    // Starting form STATE_COMBO1 -> STATE_COUNT all attacks are combos
    // So there is no need to check the ones before 
    if (can_fighter_do & CAN_COMBO)
    {
        for (uint32_t state = STATE_COMBO1; state < STATE_COUNT; state++)
        {
            attack_id_t combo_id = state_defs[state].attack;
            if (combo_id == ATK_ID_NONE) continue; 
            if (player_check_combo(player, &fighter->attacks[combo_id].sequence))
            {
                //assert(false && "TODO: Make ATK_FLAG_GRAB");
                fighter_set_state(fighter, state);
            }
        }
    }
    // ---- STATE MACHINE -------------------------------------------------------------------
    switch (fighter->state)
    {
        // ---- IDLE ------------------------------------------------------------------------
        case STATE_POSE_VICTORY: break;
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
            { 
                fighter->velocity_x = -fighter->walk_speed;
            }
            else if (input & INPUT_HOLDING_RIGHT) 
            {
                fighter->velocity_x = fighter->walk_speed;
            }
            else
                fighter_set_state(fighter, STATE_IDLE);
                
            break;
        }
        // ---- DASH ------------------------------------------------------------------------
        case STATE_DASH_FORWARD:
        {
            // On state entry (state_timer near zero) give the burst
            if (fighter->state_timer <= delta_time)
            {
                float dir = fighter->facing_right ? 1.0f : -1.0f;
                fighter->velocity_x = dir * 500.0f; // tune this
            }
        
            // End dash after duration or when velocity bleeds off
            if (fighter->state_timer >= 0.25f)
                fighter_set_state(fighter, STATE_WALK_FORWARD);
            
            break;
        }
        case STATE_DASH_BACKWARD:
        {
            // On state entry (state_timer near zero) give the burst
            if (fighter->state_timer <= delta_time)
            {
                float dir = fighter->facing_right ? -1.0f : 1.0f;
                fighter->velocity_x = dir * 500.0f; // tune this
            }
        
            // End dash after duration or when velocity bleeds off
            if (fighter->state_timer >= 0.25f)
                fighter_set_state(fighter, STATE_WALK_BACKWARD);

            break;
        }
        // ---- JUMP ------------------------------------------------------------------------
        case STATE_AIRBORNE: 
        {
            // Air steering
            if (input & INPUT_HOLDING_LEFT)       fighter->velocity_x = -fighter->walk_speed;
            else if (input & INPUT_HOLDING_RIGHT) fighter->velocity_x = fighter->walk_speed;

            break;
        }
        // ---- BLOCK -----------------------------------------------------------------------
        case STATE_STAND_BLOCK: 
        {
            fighter->velocity_x = 0.0f;
            
            if (!((swaped_input & INPUT_HOLDING_BLOCK) == INPUT_HOLDING_BLOCK))
                fighter_set_state(fighter, STATE_IDLE);

            if (input & INPUT_HOLDING_DOWN) fighter_set_state(fighter, STATE_CROUCH);
            break;
        }
        // ---- CROUCH BLOCK ----------------------------------------------------------------
        case STATE_CROUCH_BLOCK: 
        {
            fighter->velocity_x = 0.0f;
            if (!((swaped_input & INPUT_HOLDING_BLOCK) == INPUT_HOLDING_BLOCK)) 
                fighter_set_state(fighter, STATE_CROUCH);
            
            if (!(input & INPUT_HOLDING_DOWN)) fighter_set_state(fighter, STATE_STAND_BLOCK);
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

        // ---- HITSTUNS --------------------------------------------------------------------
        case STATE_STAND_HITSTUN:
        case STATE_CROUCH_HITSTUN:
        {
            if (fighter->state_timer >= fighter->active_stun_duration)
                fighter_set_state(fighter, (fighter->state == STATE_STAND_HITSTUN) ? 
                STATE_IDLE : STATE_CROUCH);
            break;
        }
        
        case STATE_AIRBORNE_HITSTUN:
        {
            if (fighter->state_timer >= fighter->active_stun_duration)
                fighter_set_state(fighter, STATE_AIRBORNE);
            break;
        }
        // ---- KNOCKDOWN -------------------------------------------------------------------
        case STATE_KNOCKDOWN:
        {
            // after half of the 'active_stun_duration' time pased it gets to a STATE_RECOVERY
            if (fighter->state_timer >= (fighter->active_stun_duration))
                fighter_set_state(fighter, STATE_RECOVERY);
            break;
        } 
        // ---- RECOVERY --------------------------------------------------------------------
        case STATE_RECOVERY:
        {
            if (fighter->state_timer >= ((float)fighter->animation->frame_count * fighter->animation->frame_duration))
            {
                if (fighter->is_grounded)
                    fighter_set_state(fighter, STATE_IDLE);
                else
                    fighter_set_state(fighter, STATE_AIRBORNE);
            }
            break;
        }     
        // ---- JUMP ATTACK -----------------------------------------------------------------
        case STATE_AIRBORNE_ATK:     
        {
            if (fighter->state_timer >= fighter->active_atk_duration)
            {
                fighter_set_state(fighter, STATE_AIRBORNE);
            }    
            break;
        }
        // ---- CROUCH ATTACK ---------------------------------------------------------------
        case STATE_CROUCH_LIGHT:
        case STATE_CROUCH_MEDIUM:
        case STATE_CROUCH_HEAVY:
        {
            if (fighter->state_timer >= fighter->active_atk_duration)
            {
                fighter_set_state(fighter, STATE_CROUCH);
            }
            break;
        }
        // ---- STAND ATTACK ----------------------------------------------------------------
        case STATE_STAND_LIGHT:  
        case STATE_STAND_MEDIUM: 
        case STATE_STAND_HEAVY:  
        case STATE_COMBO1:
        case STATE_COMBO2:
        case STATE_COMBO3:
        case STATE_SPECIAL1:
        case STATE_SPECIAL2:
        {
            if (fighter->state_timer >= fighter->active_atk_duration)
            {
                fighter_set_state(fighter, STATE_IDLE);
            }
            break; 
        } 
        case STATE_COUNT: break; // Ignore
    }
    // Animation update
    fighter_update_animation(fighter, delta_time);
}

static void fighter_update_animation(fighter_t *fighter, float delta_time)
{
    fighter->animation_timer += delta_time;

    // ---- ANIM FRAME ---------------------------------------------------
    const animation_t *anim = fighter->animation; 
    
    if (fighter->animation_timer >= anim->frame_duration)
    {
        fighter->animation_timer -= anim->frame_duration;
        fighter->animation_frame++;
    
        if (anim->frame_count <= fighter->animation_frame)
        {
            if (anim->loop)
                fighter->animation_frame = 0;
            else
                fighter->animation_frame = anim->frame_count - 1;
        }
    }
        
    // ---- ANIM STATE ---------------------------------------------------
    const animation_id_t curr_anim = fighter->animation_id;
    animation_id_t       next_anim = state_defs[fighter->state].anim;  
    bool restart = true; // restart animation 

    if (curr_anim == ANIM_AIRBORNE_ATK || curr_anim == ANIM_AIRBORNE_HITSTUN) 
        restart = false;
    if ((next_anim == ANIM_CROUCH) && (
        curr_anim == ANIM_CROUCH_LIGHT   || 
        curr_anim == ANIM_CROUCH_MEDIUM  || 
        curr_anim == ANIM_CROUCH_HEAVY   ||
        curr_anim == ANIM_CROUCH_HITSTUN ||
        curr_anim == ANIM_CROUCH_BLOCK)
        ) restart = false;

    if (next_anim == ANIM_WALK_FORWARD || next_anim == ANIM_WALK_BACKWARD) 
    {
        next_anim = (fighter->velocity_x > 0) == fighter->facing_right
            ? ANIM_WALK_FORWARD 
            : ANIM_WALK_BACKWARD;
    }

    // ---- ANIM CHANGE --------------------------------------------------
    if (curr_anim != next_anim) 
    {
        // if animation changes then it restarts the timer and the frame to zero
        fighter->animation = &fighter->visuals->animations[next_anim];
        fighter->animation_id = next_anim;
        fighter->animation_timer = 0.0f;
        // Sets the animation to last frame or first
        if (restart)
            fighter->animation_frame = 0;
        else
            fighter->animation_frame = (fighter->animation->frame_count - 1);
    }
}

// NEW

const anim_frame_t *fighter_get_frame_data(fighter_t *fighter)
{
    return &fighter->visuals->animations[fighter->animation_id].frames[fighter->animation_frame];
}

// rect.x - off.x / rect.y - off.y
SDL_Rect to_world_rect(fighter_t *fighter, SDL_Rect local)
{
    const anim_frame_t *frame = fighter_get_frame_data(fighter);

    // top-left corner of the sprite in world space
    float sprite_left, sprite_top;

    if (fighter->facing_right)
        sprite_left = fighter->position_x - (float)frame->offset_x;
    else
        // flipped: offset_x measured from right edge instead
        sprite_left = fighter->position_x - (float)(frame->src.w - frame->offset_x);

    sprite_top = fighter->position_y - (float)frame->offset_y;

    SDL_Rect world;

    if (fighter->facing_right)
    {
        world.x = (int32_t)sprite_left + local.x;
    }
    else
    {
        // mirror the box horizontally within the sprite
        // local.x is from left edge of sprite, when flipped it becomes from right edge
        world.x = (int32_t)sprite_left + (frame->src.w - local.x - local.w);
    }

    world.y = (int32_t)sprite_top  + local.y;
    world.w = local.w;
    world.h = local.h;

    return world;
}

static bool fighter_check_hit(fighter_t *atk, fighter_t *def)
{
    const anim_frame_t *col_atk = fighter_get_frame_data(atk);
    const anim_frame_t *col_def = fighter_get_frame_data(def);

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
    const anim_frame_t *f1_col = fighter_get_frame_data(f1);
    const anim_frame_t *f2_col = fighter_get_frame_data(f2);

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
                return 0.0f;
        }
    }
    return 0.0f;
}

void fighter_check_attack(fighter_t *atk, fighter_t *def, void *ctx)
{
    if (atk->curr_attack_id == ATK_ID_NONE) return;

    const attack_t    *attack = &atk->attacks[atk->curr_attack_id];
    const animation_t *anim   = atk->animation;
    int32_t            frame  = atk->animation_frame;

    // ---- ACTIVE WINDOW ---------------------------------------------------
    bool in_active_window = (frame >= (int32_t)anim->startup_frames &&
                             frame <  (int32_t)(anim->startup_frames + anim->active_frames));
    
    if (!in_active_window) return;
    if (atk->last_hit_frame == frame) return; // already hit this frame

    // ---- CONTEXT ---------------------------------------------------------
    bool crouching = (
        def->state == STATE_CROUCH        || 
        def->state == STATE_CROUCH_BLOCK  ||
        def->state == STATE_CROUCH_LIGHT  || 
        def->state == STATE_CROUCH_MEDIUM ||
        def->state == STATE_CROUCH_HEAVY);

    bool airborne = (
        def->state == STATE_AIRBORNE     ||
        def->state == STATE_AIRBORNE_ATK ||
        def->state == STATE_AIRBORNE_HITSTUN);

    bool blocked = (!airborne) && (
        (def->state == STATE_STAND_BLOCK  && !(attack->flags & ATK_FLAG_CANT_BLOCK_STANDING)) ||
        (def->state == STATE_CROUCH_BLOCK && !(attack->flags & ATK_FLAG_CANT_BLOCK_CROUCHING))
    );

    float dir = atk->facing_right ? 1.0f : -1.0f;

    bool is_last_hit = (frame == (int32_t)(anim->startup_frames + anim->active_frames - 1));
    bool is_multihit = (attack->flags & ATK_FLAG_MULTIHIT);

    // ---- GRAB ------------------------------------------------------------
    if (attack->flags & ATK_FLAG_GRAB)
    {
        float dist    = SDL_fabsf(atk->position_x - def->position_x);
        bool grabbable = (dist < 40.0f          &&
                          def->is_grounded       &&
                          !blocked               && // cant grab while blocking
                          def->curr_attack_id == ATK_ID_NONE &&
                          def->state != STATE_STAND_HITSTUN  &&
                          def->state != STATE_CROUCH_HITSTUN &&
                          def->state != STATE_KNOCKDOWN      &&
                          def->state != STATE_RECOVERY);
        if (!grabbable) return;

        atk->last_hit_frame = frame;
        def->hp            -= attack->damage;
        def->velocity_x     = dir  * attack->knockback_x;
        def->velocity_y     =        attack->knockback_y;
        def->active_stun_duration = attack->stun_duration;

        fighter_set_state(def, (attack->flags & ATK_FLAG_KNOCKDOWN)
            ? STATE_KNOCKDOWN
            : STATE_STAND_HITSTUN);

        if (attack->func) attack->func(atk, def, ctx);
        return;
    }

    // ---- TRIGGER ---------------------------------------------------------
    bool hit = fighter_check_hit(atk, def);

    switch (attack->triger)
    {
        case ATK_TRIGGER_ON_HIT:
            if (!hit) return;
            break;

        case ATK_TRIGGER_ON_COUNTER:
            if (!hit) return;
            if (def->curr_attack_id == ATK_ID_NONE) return;
            break;

        case ATK_TRIGGER_ON_WHIFF:
            if (atk->last_hit_frame != -1) return;           // something already landed
            if (atk->state_timer < atk->active_atk_duration) return; // attack still going
            atk->active_atk_duration += 0.3f;                // whiff penalty
            if (attack->func) attack->func(atk, def, ctx);
            return;

        case ATK_TRIGGER_ON_BLOCK:
            if (!hit || !blocked) return;

            atk->last_hit_frame  = frame;
            def->hp             -= (int32_t)((float)attack->damage * 0.10f);
            def->velocity_x      =  dir * 80.0f;
            atk->velocity_x     += -(dir * 40.0f);

            if (attack->func) attack->func(atk, def, ctx);
            return; // defender stays in block state, no hitstun
    }

    // ---- BLOCKED HIT (for ON_HIT attacks that get blocked) ---------------
    if (blocked)
    {
        atk->last_hit_frame  = frame;
        def->hp             -= (int32_t)((float)attack->damage * 0.10f);
        def->velocity_x      =  dir * 80.0f;
        atk->velocity_x     += -(dir * 40.0f);

        if (attack->func) attack->func(atk, def, ctx);
        return;
    }

    // ---- APPLY HIT -------------------------------------------------------
    atk->last_hit_frame = frame;

    // recoil always applies
    atk->velocity_x += -(dir * attack->recoil_x);
    atk->velocity_y +=       -attack->recoil_y;

    if (is_multihit && !is_last_hit)
    {
        // ---- INTERMEDIATE HIT --------------------------------------------
        def->hp             -= attack->damage;
        def->velocity_x      = dir * 20.0f;  // small push, keeps in range
        def->velocity_y      = 0.0f;
        def->active_stun_duration = 0.04f;

        fighter_set_state(def, crouching ? STATE_CROUCH_HITSTUN : STATE_STAND_HITSTUN);
        if (airborne) fighter_set_state(def, STATE_AIRBORNE_HITSTUN);
    }
    else
    {
        // ---- FINAL / SINGLE HIT ------------------------------------------
        def->hp             -= attack->damage;
        def->velocity_x      = dir  * attack->knockback_x;
        def->velocity_y      =        attack->knockback_y;
        def->active_stun_duration = attack->stun_duration;

        if (attack->flags & ATK_FLAG_KNOCKDOWN)
            fighter_set_state(def, STATE_KNOCKDOWN);
        else
        {
            fighter_set_state(def, crouching ? STATE_CROUCH_HITSTUN : STATE_STAND_HITSTUN);
            if (airborne) fighter_set_state(def, STATE_AIRBORNE_HITSTUN);
        }
    }

    if (attack->func) attack->func(atk, def, ctx);
}


#endif /* FAJTER_IMPLEMENTATION */

#endif /* !_FAJTER_H */
