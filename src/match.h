#ifndef _MATCH_H
#define _MATCH_H

#include <stdint.h>
#include "assets.h"
#include "camera.h"
#include "player.h" 

struct renderer_t;
struct player_t;

typedef struct stage_t 
{
    int32_t w, h; // Width & Height of stage
    int32_t texture;
    int32_t floor_level;
} stage_t;

typedef struct match_resurces_t
{
    int32_t ui;
    stage_t stage; 
} match_resurces_t;

typedef struct match_palyer_info_t
{
    int32_t score;
    int32_t base_hp; // HP default of fighters 
    float   percent_hp;  // HP in %
    bool    was_hit; // Has attack landed
    float   hit_position_y; // Uesd for correct sparcks position
    bool    blocked; // Has attack been blocked
    bool    walled;
    bool    wall_bouncing; // if true the palyer will get bounced if it gets walled 
} match_palyer_info_t;

typedef enum match_state_t 
{
    MATCH_STATE_PLAY = 0,
    MATCH_STATE_START,
    MATCH_STATE_END,
    MATCH_STATE_VICTORY_P1,
    MATCH_STATE_VICTORY_P2,
    MATCH_STATE_EXIT /** The caller of match_update() should perform exit */
} match_state_t;

typedef struct match_t 
{
    match_state_t state;
    float state_timer;
    
    float round_duration;
    float round_timer;
    int32_t rounds;

    camera_t camera;
    match_resurces_t res;
    
    int32_t hitstop;
    // Palyer starts
    struct player_t *p1, *p2;
    match_palyer_info_t f1, f2;
    // NOTE: Used for landing particle for now
    struct {
        animated_object_t items[4];
        uint8_t count;
    } particles;
} match_t;

#define FLOOR_MARGINE 30
#define WALL_MARGINE  30
#define UI_TILE_SIZE  64 
#define UI_TILE(idx) tile_from_atlas(idx, UI_TILE_SIZE, UI_TILE_SIZE, 8) 

match_t match_start(renderer_t *renderer, float duration, int32_t rounds, struct player_t *p1, struct player_t *p2, match_resurces_t *textures);
void match_update(match_t *match, float delta_time);
void match_render(const match_t *match, struct renderer_t *renderer);

stage_t stage_load(renderer_t *renderer, asset_name_t stage);
void stage_unload(renderer_t *renderer, stage_t *stage);

#ifdef MATCH_IMPLEMENTATION
#include "renderer.h"
#include "characters.h"

// TODO: Move to somewhere good, or even better get gud
global_variable animation_def_t match_ui_attack_sparcle_anim[2] = 
{
    // Succesfull land
    [0] = ANIM(false, TICKS(6),
        FRAME_IMMUNE(UI_TILE(8),  TICKS(2), UI_TILE_SIZE/2, UI_TILE_SIZE/2),
        FRAME_IMMUNE(UI_TILE(9),  TICKS(3), UI_TILE_SIZE/2, UI_TILE_SIZE/2),
        FRAME_IMMUNE(UI_TILE(10), TICKS(1), UI_TILE_SIZE/2, UI_TILE_SIZE/2),
    ),
    // Blocked attack
    [1] = ANIM(false, TICKS(6),
        FRAME_IMMUNE(UI_TILE(11), TICKS(4), UI_TILE_SIZE/2, UI_TILE_SIZE/2),
        FRAME_IMMUNE(UI_TILE(12), TICKS(5), UI_TILE_SIZE/2, UI_TILE_SIZE/2),
        FRAME_IMMUNE(UI_TILE(13), TICKS(2), UI_TILE_SIZE/2, UI_TILE_SIZE/2),
    )
};

match_t match_start(renderer_t *renderer, float duration, int32_t rounds, player_t *p1, player_t *p2, match_resurces_t *textures)
{
    (void)renderer;

    match_t match = 
    {
        .state = MATCH_STATE_START,
        .round_duration = duration,
        .round_timer    = duration,
        .rounds         = rounds,
        .p1 = p1, .p2 = p2,
        
        .res = *textures,

        .f1.base_hp = p1->fighter.hp,
        .f2.base_hp = p2->fighter.hp,
        
        .f1.percent_hp = 1.0f,
        .f2.percent_hp = 1.0f,
    };

    match.p1->fighter.pysics.position.x = (float)(match.res.stage.w / 2) - 50;
    match.p1->fighter.pysics.position.y = (float)SCREEN_HEIGHT / 2;
    
    match.p2->fighter.pysics.position.x = (float)(match.res.stage.w / 2) + 50;
    match.p2->fighter.pysics.position.y = (float)SCREEN_HEIGHT / 2;

    return match;
}

internal void match_set_state(match_t *match, match_state_t state)
{
    match->state = state; 
    match->state_timer = 0.0f;
}

internal void match_fighter_apply_events(match_t *match, fighter_t *fighter, match_palyer_info_t *info)
{
    // Wall bouncing logic
    info->walled = !is_in_range(
        (match->camera.view.x) + WALL_MARGINE, 
        (match->camera.view.x + match->camera.view.w) - WALL_MARGINE, 
        fighter->pysics.position.x
    );

    if (info->wall_bouncing) 
    {
        if (info->walled)
        {
            info->wall_bouncing = false;
            
            camera_shake_trigger(&match->camera, 10.0f, 0.5f);
            fighter->pysics.velocity.x *= -1.0f;    
        }
        if (!is_fighter_stuned(fighter))
            info->wall_bouncing = false;
    }

    fighter->pysics.position.x = 
        SDL_clamp(fighter->pysics.position.x, 
            (float)match->camera.view.x, 
            (float)(match->camera.view.x + match->camera.view.w)
    );

    // Spwan attacking sparcks
    if (info->blocked | info->was_hit) {
        int32_t anim_id = info->blocked ? 1 : 0; 
        info->blocked = false;
        info->was_hit = false;

        animated_object_t anim_obj = {
            .position     = vec2f(fighter->pysics.position.x, info->hit_position_y),
            .facing_right = fighter->pysics.facing_right, 
        };
        animation_init(&anim_obj.anim, match->res.ui, &match_ui_attack_sparcle_anim[anim_id]);
        stack_push(&match->particles, anim_obj);   
    }

    // Update sparcks
    for_range_i(match->particles.count)
    {
        if (match->particles.items[i].anim.timer < match->particles.items[i].anim.duration)
            animation_update(&match->particles.items[i].anim);
        else
            stack_pop_unordered_at(&match->particles, i);
    }
}

internal void match_enforce_rules(match_t *match, bool record_input, float delta_time)
{
    // Fighters allways facing each other
    fighter_t *f1 = &match->p1->fighter;
    fighter_t *f2 = &match->p2->fighter;

    pysics_facing_direction(&f1->pysics, &f2->pysics);
    if (record_input)
    {
        player_record_input(match->p1, delta_time);
        player_record_input(match->p2, delta_time);
    }
    else 
    {
        match->p1->input_history[lenghtof(match->p1->input_history) - 1] = 0;
        match->p2->input_history[lenghtof(match->p2->input_history) - 1] = 0;
    } 
    
    float overlap = fighter_check_overlap(f1, f2); 
    if (overlap > 0.0f)
    {
        // P1 
        float dir = f1->pysics.facing_right ? -1.0f : 1.0f;
        float total_vel = SDL_fabsf(f1->pysics.velocity.x) + SDL_fabsf(f2->pysics.velocity.x);
        
        float f1_pushed = (total_vel > 0.0f)
            ?  SDL_fabsf(f1->pysics.velocity.x) / total_vel
            : 0.5f;
        float f2_pushed = 1.0f - f1_pushed;

        if (match->f1.walled)      { f1_pushed = 0.0f; f2_pushed = 1.0f; }
        else if (match->f2.walled) { f1_pushed = 1.0f; f2_pushed = 0.0f; }

        f1->pysics.position.x +=  (dir * overlap * f1_pushed);
        f2->pysics.position.x += -(dir * overlap * f2_pushed);
    }
    
    match_fighter_apply_events(match, f1, &match->f1);
    match_fighter_apply_events(match, f2, &match->f2);
}

void match_update(match_t *match, float delta_time)
{
    match->state_timer += delta_time;

    player_t *p1 = match->p1;
    player_t *p2 = match->p2;

    camera_update(
        &match->camera, 
        &p1->fighter.pysics, &p2->fighter.pysics, 
        vec2i(match->res.stage.w, match->res.stage.h), 
        delta_time
    );

    match_enforce_rules(match, true, delta_time);

    float dt = (match->state == MATCH_STATE_END) ? delta_time / 2 : delta_time; 
    if (match->hitstop > 0)
    {
        match->hitstop--;
        return;
    }

    fighter_update(p1, &p1->fighter, dt, match->res.stage.floor_level);
    fighter_update(p2, &p2->fighter, dt, match->res.stage.floor_level);
    
    switch (match->state)
    {
        case MATCH_STATE_PLAY:
        {
            match->round_timer -= delta_time;

            match->f1.percent_hp = ((float)p1->fighter.hp / (float)match->f1.base_hp);
            match->f2.percent_hp = ((float)p2->fighter.hp / (float)match->f2.base_hp);

            // Game ends on hp or time loss
            if (match->round_timer <= 0.0f ||
                p1->fighter.hp <= 0 || p2->fighter.hp <= 0)
            {
                p1->fighter.hp = (p1->fighter.hp < 0) ? 0 : p1->fighter.hp; 
                p2->fighter.hp = (p2->fighter.hp < 0) ? 0 : p2->fighter.hp; 
                
                if (match->f1.percent_hp > match->f2.percent_hp)      
                { 
                    match->f1.score++;
                    match->rounds--;

                    fighter_set_state(&p1->fighter, STATE_POSE_VICTORY);
                    fighter_set_state(&p2->fighter, STATE_POSE_DEFEAT);
                }
                else if (match->f1.percent_hp < match->f2.percent_hp) 
                { 
                    match->f2.score++;
                    match->rounds--;

                    fighter_set_state(&p2->fighter, STATE_POSE_VICTORY);
                    fighter_set_state(&p1->fighter, STATE_POSE_DEFEAT);
                }
                    
                match_set_state(match, MATCH_STATE_END);
            }
            
            fighter_update_attack(&p1->fighter, &p2->fighter, match);
            fighter_update_attack(&p2->fighter, &p1->fighter, match);
            
            for_range_i(lenghtof(p1->fighter.projectiles))
            {
                if (p1->fighter.projectiles[i].active) 
                    projectile_update_attack(&p1->fighter.projectiles[i], &p2->fighter, match);

                if (p2->fighter.projectiles[i].active) 
                    projectile_update_attack(&p2->fighter.projectiles[i], &p1->fighter, match);
            }
            break;
        }
        case MATCH_STATE_START:
        {
            if (match->state_timer >= 2.0f)
            {
                match->round_timer = match->round_duration;
                p1->fighter.hp = match->f1.base_hp;    
                p2->fighter.hp = match->f2.base_hp;

                fighter_set_state(&p1->fighter, STATE_IDLE);
                fighter_set_state(&p2->fighter, STATE_IDLE);
                
                match_set_state(match, MATCH_STATE_PLAY);
                
                if (match->rounds <= 0) 
                {
                    if (match->f1.score > match->f2.score) 
                    {
                        match_set_state(match, MATCH_STATE_VICTORY_P1);
                    }

                    else if (match->f1.score < match->f2.score) 
                    {
                        match_set_state(match, MATCH_STATE_VICTORY_P2);
                    }    
                }  
            }

            //fighter_projectile_update(&p1->fighter, &p2->fighter, delta_time);
            //fighter_projectile_update(&p2->fighter, &p1->fighter, delta_time);
            
            break;
        }

        case MATCH_STATE_END:
        {
            if (match->state_timer >= 3.0f)
            {
                match_set_state(match, MATCH_STATE_START);
            }

            //fighter_projectile_update(&p1->fighter, &p2->fighter, delta_time);
            //fighter_projectile_update(&p2->fighter, &p1->fighter, delta_time);
            
            break;
        }

        case MATCH_STATE_VICTORY_P1:
        case MATCH_STATE_VICTORY_P2:
        {
            if (match->state_timer >= 5.0f && match->state_timer <= 6.0f)
            {
                if (match->state == MATCH_STATE_VICTORY_P1)
                {    
                    fighter_set_state(&p1->fighter, STATE_POSE_VICTORY);
                    fighter_set_state(&p2->fighter, STATE_POSE_DEFEAT);
                } else
                {
                    fighter_set_state(&p2->fighter, STATE_POSE_VICTORY);
                    fighter_set_state(&p1->fighter, STATE_POSE_DEFEAT);
                }
            }

            //fighter_projectile_update(&p1->fighter, &p2->fighter, delta_time);
            //fighter_projectile_update(&p2->fighter, &p1->fighter, delta_time);
            
            break;
        }

        case MATCH_STATE_EXIT: break;
    }
} 

internal void draw_ui_overlay(const match_t *match, renderer_t *renderer);

void match_render(const match_t *match, renderer_t *renderer)
{
    player_t *p1 = match->p1; 
    player_t *p2 = match->p2; 

    draw_ui_overlay(match, renderer);

    if (p2->fighter.curr_attack_id != ATK_ID_NONE) {
        renderer_draw_fighter(renderer, &p1->fighter, match->camera.view, match->res.stage.floor_level);
        renderer_draw_fighter(renderer, &p2->fighter, match->camera.view, match->res.stage.floor_level);
    } else
    {
        renderer_draw_fighter(renderer, &p2->fighter, match->camera.view, match->res.stage.floor_level);
        renderer_draw_fighter(renderer, &p1->fighter, match->camera.view, match->res.stage.floor_level);
    }
    // projectile rendereing
    for_range_i(lenghtof(p1->fighter.projectiles))
    {
        if (p1->fighter.projectiles[i].active)
            renderer_draw_projectile(renderer, &p1->fighter.projectiles[i], match->camera.view, match->res.stage.floor_level);
        
        if (p2->fighter.projectiles[i].active)
            renderer_draw_projectile(renderer, &p2->fighter.projectiles[i], match->camera.view, match->res.stage.floor_level);
    }

    for_range_i(match->particles.count)
    {
        renderer_draw_animation(renderer, LAYER_UI1, &match->particles.items[i], match->camera.view);
    }
    
    //static char buff[128] = "";
    //#define X(name) (p1->fighter.state.id == STATE_##name) ? #name :     
    //snprintf(buff, sizeof(buff), 
    //    "\n\n\nP1:%f       P2:%f\n%s\nDUR:%d TICK:%d", 
    //    (double)p1->fighter.pysics.position.x, 
    //    (double)p2->fighter.pysics.position.x,
    //    FIGHTER_STATE_NAMES_XLIST "None",
    //    p1->fighter.state.duration,
    //    p1->fighter.state.timer
    //);
    //#undef X
    //renderer_draw_text(renderer, LAYER_UI1, (const char *)buff, 21, 21, 20, 20, COLOR_BLACK);
    //renderer_draw_text(renderer, LAYER_UI1, (const char *)buff, 20, 20, 20, 20, COLOR_WHITE);
}

internal void draw_relative_texture_width(
    renderer_t *renderer, 
    render_layer_t layer,
    texture_handle_t handle,
    const SDL_Rect *src, 
    const SDL_Rect *dst, 
    float percentage)
{
    SDL_Rect src_re = *src; 
    SDL_Rect dst_re = *dst;
    src_re.w = (int32_t)((float)src_re.w * percentage);
    dst_re.w = (int32_t)((float)dst_re.w * percentage);

    renderer_draw_texture(
        renderer, 
        layer, 
        handle, 
        &src_re,
        &dst_re,
        0.0, 
        SDL_FLIP_NONE
    );
}

internal void draw_ui_overlay(const match_t *match, renderer_t *renderer)
{
    // Index position in 'ui_bar' atlas
    SDL_Rect GEAR    = UI_TILE(0);
    SDL_Rect END_BAR = UI_TILE(1);
    SDL_Rect BAR     = UI_TILE(2);
    SDL_Rect KEBAB   = UI_TILE(3);
    SDL_Rect RAGEBAIT_METER = UI_TILE(4);

    SDL_Rect rage_dst = {0, SCREEN_HEIGHT - UI_TILE_SIZE, UI_TILE_SIZE, UI_TILE_SIZE};

    draw_relative_texture_width(
        renderer, 
        LAYER_UI1,
        match->res.ui,
        &RAGEBAIT_METER, 
        &rage_dst, 
        SDL_clamp((float)match->p1->fighter.ragebait_meter / 300.0f, 0.0f, 1.0f)
    );

    SDL_Rect end_bar1 = {0, 0, UI_TILE_SIZE, UI_TILE_SIZE};

    renderer_draw_texture(
        renderer, 
        LAYER_UI1, 
        match->res.ui, 
        &END_BAR,
        &end_bar1,
        0.0, 
        SDL_FLIP_NONE
    );

    SDL_Rect bar = {UI_TILE_SIZE, 0, SCREEN_WIDTH - (UI_TILE_SIZE * 2), UI_TILE_SIZE}; 

    renderer_draw_texture(
        renderer, 
        LAYER_UI1, 
        match->res.ui, 
        &BAR,
        &bar,
        0.0, 
        SDL_FLIP_NONE
    );

    SDL_Rect end_bar2 = {bar.x + bar.w, 0, UI_TILE_SIZE, UI_TILE_SIZE};
    renderer_draw_texture(
        renderer, 
        LAYER_UI1, 
        match->res.ui, 
        &END_BAR,
        &end_bar2,
        0.0, 
        SDL_FLIP_HORIZONTAL
    );

    // GEAR
    SDL_Rect gear = {
        (SCREEN_WIDTH / 2) - (UI_TILE_SIZE / 2),
        0,
        UI_TILE_SIZE,
        UI_TILE_SIZE
    };

    renderer_draw_texture(
        renderer, 
        LAYER_UI1, 
        match->res.ui, 
        &GEAR,
        &gear,
        0.0, 
        SDL_FLIP_NONE
    );
    
    
    SDL_Rect p1_kebab_bar = {UI_TILE_SIZE, 0, gear.x - UI_TILE_SIZE, UI_TILE_SIZE};
    
    float offset1 = match->f1.percent_hp < 0.0f ? 0.0f : match->f1.percent_hp;
    
    p1_kebab_bar.x += p1_kebab_bar.w - (int32_t)((float)p1_kebab_bar.w * offset1);
    p1_kebab_bar.w = (int32_t)((float)p1_kebab_bar.w * offset1);

    renderer_draw_texture(
        renderer, 
        LAYER_UI1, 
        match->res.ui, 
        &KEBAB,
        &p1_kebab_bar,
        0.0, 
        SDL_FLIP_NONE
    );

    SDL_Rect p2_kebab_bar = {gear.x + gear.w, 0, (SCREEN_WIDTH / 2) - (UI_TILE_SIZE + (gear.w / 2)), UI_TILE_SIZE};
    
    float offset2 = match->f2.percent_hp < 0.0f ? 0.0f : match->f2.percent_hp;
    p2_kebab_bar.w = (int32_t)((float)p2_kebab_bar.w * offset2);
    
    renderer_draw_texture(
        renderer, 
        LAYER_UI1, 
        match->res.ui, 
        &KEBAB,
        &p2_kebab_bar,
        0.0, 
        SDL_FLIP_HORIZONTAL
    );

    // --- BAR -----
    static char nih[8] = "";
    snprintf(nih, sizeof(nih), "%02d", (int32_t)match->round_timer);
    renderer_draw_text(renderer, LAYER_UI1, nih, (SCREEN_WIDTH / 2) - 15, 25, 15, 15, COLOR_WHITE);
    
    // Background
    renderer_draw_texture(renderer, LAYER_BACKGROUND, match->res.stage.texture, &match->camera.view, NULL, 0.0, SDL_FLIP_NONE);
}

stage_t stage_load(renderer_t *renderer, asset_name_t stage_name)
{
    assert(is_in_range(0, ASSET_COUNT, stage_name) && "Stage not found: OOB");

    stage_t stage = {0};
    if (asset_load(&global_assets[stage_name], renderer))
    {
        renderer_texture_size(renderer, global_assets[stage_name].handle.texture, &stage.w, &stage.h);
        stage.texture = global_assets[stage_name].handle.texture;
        stage.floor_level = stage.h - FLOOR_MARGINE;
    }
    
    return stage;
}

void stage_unload(renderer_t *renderer, stage_t *stage)
{
    asset_unload(&global_assets[stage->texture], renderer);
    *stage = (stage_t){0};
}

#endif // MATCH_IMPLEMENTATION

#endif // !_MATCH_H
