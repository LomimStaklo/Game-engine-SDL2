#ifndef _MATCH_H
#define _MATCH_H

#include <stdint.h>
#include "assets.h"
#include "camera.h"

struct renderer_t;
struct player_t;

typedef struct match_resurces_t
{
    int32_t ui, p1_atlas, p2_atlas, stage; 
} match_resurces_t;

typedef struct match_palyer_info_t
{
    int32_t score;
    int32_t base_hp; // HP default of fighters 
    float   percent_hp;  // HP in %
    bool    walled;
    bool    walled_;
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
    vec2i_t stage_size;

    match_resurces_t textures;
    
    int32_t hitstop;
    // Palyer starts
    struct player_t *p1, *p2;
    match_palyer_info_t f1, f2;
} match_t;

#define FLOOR_MARGINE 30

match_t match_start(renderer_t *renderer, float duration, int32_t rounds, struct player_t *p1, struct player_t *p2, match_resurces_t *textures);
void match_update(match_t *match, float delta_time);
void match_render(const match_t *match, struct renderer_t *renderer);

#ifdef MATCH_IMPLEMENTATION
#include "renderer.h"
#include "player.h" 
#include "characters.h"

match_t match_start(renderer_t *renderer, float duration, int32_t rounds, player_t *p1, player_t *p2, match_resurces_t *textures)
{
    match_t match = 
    {
        .state = MATCH_STATE_START,
        .round_duration = duration,
        .round_timer    = duration,
        .rounds         = rounds,
        .p1 = p1, .p2 = p2,
        
        .textures = *textures,

        .f1.base_hp = p1->fighter.hp,
        .f2.base_hp = p2->fighter.hp,
        
        .f1.percent_hp = 1.0f,
        .f2.percent_hp = 1.0f,
    };

    renderer_texture_size(
        renderer, match.textures.stage,
        &match.stage_size.x, &match.stage_size.y
    );

    match.p1->fighter.pysics.position.x = (SCREEN_WIDTH / 2) - 20;
    match.p1->fighter.pysics.position.y = SCREEN_HEIGHT / 2;
    
    match.p2->fighter.pysics.position.x = (SCREEN_WIDTH / 2) + 20;
    match.p2->fighter.pysics.position.y = SCREEN_HEIGHT / 2;

    return match;
}

static void match_set_state(match_t *match, match_state_t state)
{
    match->state = state; 
    match->state_timer = 0.0f;
}

static void match_enforce_rules(match_t *match, bool record_input, float delta_time)
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

        bool p1_walled = (
            f1->pysics.position.x <= (float)match->camera.view.x + 10.0f ||  
            f1->pysics.position.x >= (float)(match->camera.view.x + match->camera.view.w) - 10.0f
        );
        
        bool p2_walled = (
            f2->pysics.position.x <= (float)match->camera.view.x + 10.0f || 
            f2->pysics.position.x >= (float)(match->camera.view.x + match->camera.view.w) - 10.0f
        );
    
        if (p1_walled)      { f1_pushed = 0.0f; f2_pushed = 1.0f; }
        else if (p2_walled) { f1_pushed = 1.0f; f2_pushed = 0.0f; }

        f1->pysics.position.x +=  (dir * overlap * f1_pushed);
        f2->pysics.position.x += -(dir * overlap * f2_pushed);
    }
    
    // Prevent walking off screen  
    f1->pysics.position.x = 
        SDL_clamp(f1->pysics.position.x, 
            (float)match->camera.view.x + 10.0f, 
            (float)(match->camera.view.x + match->camera.view.w) - 10.0f
    );
    f2->pysics.position.x = 
        SDL_clamp(f2->pysics.position.x, 
            (float)match->camera.view.x + 10.0f, 
            (float)(match->camera.view.x + match->camera.view.w) - 10.0f
    );
}

void match_update(match_t *match, float delta_time)
{
    match->state_timer += delta_time;

    player_t *p1 = match->p1;
    player_t *p2 = match->p2;

    camera_update(
        &match->camera, 
        &p1->fighter.pysics, &p2->fighter.pysics, 
        match->stage_size, 
        delta_time
    );

    int32_t floor_level = match->stage_size.y - FLOOR_MARGINE;
    match_enforce_rules(match, true, delta_time);

    float dt = (match->state == MATCH_STATE_END) ? delta_time / 2 : delta_time; 
    fighter_update(p1, &p1->fighter, dt, floor_level);
    fighter_update(p2, &p2->fighter, dt, floor_level);

    switch (match->state)
    {
        case MATCH_STATE_PLAY:
        {
            if (match->hitstop > 0)
            {
                match->hitstop--;
                return;
            }

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

static void draw_dbg_boxes(renderer_t *r, fighter_t *f, SDL_Color c, SDL_Rect view);
static void draw_ui_overlay(const match_t *match, renderer_t *renderer);

void match_render(const match_t *match, renderer_t *renderer)
{
    player_t *p1 = match->p1; 
    player_t *p2 = match->p2; 

    draw_ui_overlay(match, renderer);

    static char buff[128] = "";
    static char nih[8] = "";

    // --- BAR -----
    snprintf(nih, sizeof(nih), "%02d", (int32_t)match->round_timer);
    renderer_draw_text(renderer, LAYER_UI1, nih, (SCREEN_WIDTH / 2) - 18, 40, 20, 20, COLOR_WHITE);

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
    
    renderer_draw_text(renderer, LAYER_UI1, (const char *)buff, 21, 21, 20, 20, COLOR_BLACK);
    renderer_draw_text(renderer, LAYER_UI1, (const char *)buff, 20, 20, 20, 20, COLOR_WHITE);

    draw_dbg_boxes(renderer, &p1->fighter, COLOR_RED, match->camera.view); 
    draw_dbg_boxes(renderer, &p2->fighter, COLOR_BLUE, match->camera.view);
    
    if (p2->fighter.curr_attack_id != ATK_ID_NONE) {
        renderer_draw_fighter(renderer, &p1->fighter, match->camera.view, match->stage_size.y - FLOOR_MARGINE);
        renderer_draw_fighter(renderer, &p2->fighter, match->camera.view, match->stage_size.y - FLOOR_MARGINE);
    } else
    {
        renderer_draw_fighter(renderer, &p2->fighter, match->camera.view, match->stage_size.y - FLOOR_MARGINE);
        renderer_draw_fighter(renderer, &p1->fighter, match->camera.view, match->stage_size.y - FLOOR_MARGINE);
    }
    // projectile rendereing
    //for_range_i(lenghtof(p1->fighter.projectiles))
    //{
    //    if (p1->fighter.projectiles[i].active)
    //        renderer_draw_projectile(renderer, &p1->fighter.projectiles[i]);
    //    
    //    if (p2->fighter.projectiles[i].active)
    //        renderer_draw_projectile(renderer, &p2->fighter.projectiles[i]);
    //}
}

static void draw_ui_overlay(const match_t *match, renderer_t *renderer)
{
    // Index position in 'ui_bar' atlas
    SDL_Rect GEAR    = tile_from_atlas(0, 96, 96, 4);
    SDL_Rect END_BAR = tile_from_atlas(1, 96, 96, 4);
    SDL_Rect BAR     = tile_from_atlas(2, 96, 96, 4);
    SDL_Rect KEBAB   = tile_from_atlas(3, 96, 96, 4);
    
    SDL_Rect end_bar1 = {0, 0, 96, 96};
    
    renderer_draw_texture(
        renderer, 
        LAYER_UI1, 
        match->textures.ui, 
        &END_BAR,
        &end_bar1,
        0.0, 
        SDL_FLIP_NONE
    );

    SDL_Rect bar = {96, 0, SCREEN_WIDTH - 192, 96}; 

    renderer_draw_texture(
        renderer, 
        LAYER_UI1, 
        match->textures.ui, 
        &BAR,
        &bar,
        0.0, 
        SDL_FLIP_NONE
    );

    SDL_Rect end_bar2 = {bar.x + bar.w, 0, 96, 96};
    
    renderer_draw_texture(
        renderer, 
        LAYER_UI1, 
        match->textures.ui, 
        &END_BAR,
        &end_bar2,
        0.0, 
        SDL_FLIP_HORIZONTAL
    );

    SDL_Rect p1_kebab_bar = {96 - 72, 0, (SCREEN_WIDTH / 2) - 144 + 64, 96};
    
    float offset1 = match->f1.percent_hp < 0.0f ? 0.0f : match->f1.percent_hp;
    
    p1_kebab_bar.x += p1_kebab_bar.w - (int32_t)((float)p1_kebab_bar.w * offset1);
    p1_kebab_bar.w = (int32_t)((float)p1_kebab_bar.w * offset1);

    renderer_draw_texture(
        renderer, 
        LAYER_UI1, 
        match->textures.ui, 
        &KEBAB,
        &p1_kebab_bar,
        0.0, 
        SDL_FLIP_NONE
    );

    SDL_Rect gear = {
        (SCREEN_WIDTH / 2) - 48,
        0,
        96,
        96
    };

    renderer_draw_texture(
        renderer, 
        LAYER_UI1, 
        match->textures.ui, 
        &GEAR,
        &gear,
        0.0, 
        SDL_FLIP_NONE
    );

    SDL_Rect p2_kebab_bar = {gear.x + gear.w, 0, (SCREEN_WIDTH / 2) - 144 + 64, 96};
    
    float offset2 = match->f2.percent_hp < 0.0f ? 0.0f : match->f2.percent_hp;
    p2_kebab_bar.w = (int32_t)((float)p2_kebab_bar.w * offset2);
    
    renderer_draw_texture(
        renderer, 
        LAYER_UI1, 
        match->textures.ui, 
        &KEBAB,
        &p2_kebab_bar,
        0.0, 
        SDL_FLIP_HORIZONTAL
    );
    
    // SDL_Rect stage_view = camera_get_view_rect(&match->camera, match->stage_size);
    renderer_draw_texture(renderer, LAYER_BACKGROUND, match->textures.stage, &match->camera.view, NULL, 0.0, SDL_FLIP_NONE);
    //tile_from_atlas(GEAR, 96, 96, 4);
    //tile_from_atlas(BAR, 96, 96, 4);

}

static void draw_dbg_boxes(renderer_t *r, fighter_t *f, SDL_Color c, SDL_Rect view)
{
    const frame_t *col_f = animation_get_frame(&f->animation);

    for_range_j(col_f->count_hurtboxs)
    {
        SDL_Rect hurt = to_world_rect(f, col_f->hurtboxs[j]);
        hurt = world_to_screen_rect(view, hurt);
        renderer_draw_rect(r, LAYER_UI1, &hurt, COLOR_GREEN, false);
    }
    for_range_i(col_f->count_hitboxs)
    {
        SDL_Rect hit = to_world_rect(f, col_f->hitboxs[i]);
        hit = world_to_screen_rect(view, hit);
        renderer_draw_rect(r, LAYER_UI1, &hit, COLOR_RED, false);
    }
    //SDL_Point p1 = {(int32_t)f->pysics.position.x, (int32_t)f->pysics.position.y}, 
    //          p2 = {(int32_t)(f->pysics.position.x + (f->pysics.velocity.x * 0.16f)),
    //                (int32_t)(f->pysics.position.y + (f->pysics.velocity.y * 0.16f))};
    //renderer_draw_line(r, LAYER_UI1, p1, p2, c);
}

#endif // MATCH_IMPLEMENTATION

#endif // !_MATCH_H
