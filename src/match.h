#ifndef _MATCH_H
#define _MATCH_H

#include <stdint.h>
#include "assets.h"

struct renderer_t;
struct player_t;

typedef struct match_textures_t
{
    int32_t ui, p1_atlas, p2_atlas, stage; 
} match_textures_t;


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

    match_textures_t textures;
    
    // Palyer starts
    struct player_t *p1, *p2;
    int32_t score_p1, score_p2;
    int32_t f1_full_hp, f2_full_hp; // HP default of fighters 
    float   f1_hp_per,  f2_hp_per;  // HP in percentages
} match_t;

match_t match_start(float duration, int32_t rounds, struct player_t *p1, struct player_t *p2, match_textures_t *textures);
void match_update(match_t *match, float delta_time);
void match_render(const match_t *match, struct renderer_t *renderer);

#ifdef MATCH_IMPLEMENTATION
#include "renderer.h"
#include "player.h" 
#include "characters.h"

match_t match_start(float duration, int32_t rounds, player_t *p1, player_t *p2, match_textures_t *textures)
{
    match_t match = 
    {
        .state = MATCH_STATE_START,
        .round_duration = duration,
        .round_timer    = duration,
        .rounds         = rounds,
        .p1 = p1, .p2 = p2,
        
        .textures = *textures,

        .f1_full_hp = p1->fighter.hp,
        .f2_full_hp = p2->fighter.hp,
        
        .f1_hp_per = 1.0f,
        .f2_hp_per = 1.0f,
    };

    match.p1->fighter.position.x = (SCREEN_WIDTH / 2) - 20;
    match.p1->fighter.position.y = SCREEN_HEIGHT / 2;
    
    match.p2->fighter.position.x = (SCREEN_WIDTH / 2) + 20;
    match.p2->fighter.position.y = SCREEN_HEIGHT / 2;

    return match;
}

static void match_set_state(match_t *match, match_state_t state)
{
    match->state = state; 
    match->state_timer = 0.0f;
}

static void match_enforce_rules(player_t *p1, player_t *p2, bool record_input, float delta_time)
{
    // Fighters allways facing each other
    p1->fighter.facing_right = (p1->fighter.position.x < p2->fighter.position.x) 
        ? true 
        : false; 
    p2->fighter.facing_right = !p1->fighter.facing_right;

    if (record_input)
    {
        player_record_input(p1, delta_time);
        player_record_input(p2, delta_time);
    }
    else 
    {
        p1->input_history[lenghtof(p1->input_history) - 1] = 0;
        p2->input_history[lenghtof(p2->input_history) - 1] = 0;
    } 
    
    float overlap = fighter_check_overlap(&p1->fighter, &p2->fighter); 
    if (overlap > 0.0f)
    {
        // P1 
        float dir = p1->fighter.facing_right ? -1.0f : 1.0f;
        float total_vel = SDL_fabsf(p1->fighter.velocity.x) + SDL_fabsf(p2->fighter.velocity.x);
        
        float f1_pushed = (total_vel > 0.0f)
            ?  SDL_fabsf(p1->fighter.velocity.x) / total_vel
            : 0.5f;
        float f2_pushed = 1.0f - f1_pushed;

        bool p1_walled = (p1->fighter.position.x <= 10.0f || p1->fighter.position.x >= SCREEN_WIDTH - 10);
        bool p2_walled = (p2->fighter.position.x <= 10.0f || p2->fighter.position.x >= SCREEN_WIDTH - 10);
    
        if (p1_walled)      { f1_pushed = 0.0f; f2_pushed = 1.0f; }
        else if (p2_walled) { f1_pushed = 1.0f; f2_pushed = 0.0f; }

        p1->fighter.position.x +=  (dir * overlap * f1_pushed);
        p2->fighter.position.x += -(dir * overlap * f2_pushed);
    }
    
    // Prevent walking off screen  
    p1->fighter.position.x = SDL_clamp(p1->fighter.position.x, 10.0f, (float)SCREEN_WIDTH - 10.0f);
    p2->fighter.position.x = SDL_clamp(p2->fighter.position.x, 10.0f, (float)SCREEN_WIDTH - 10.0f);
}

void match_update(match_t *match, float delta_time)
{
    match->state_timer += delta_time;

    player_t *p1 = match->p1;
    player_t *p2 = match->p2;

    switch (match->state)
    {
        case MATCH_STATE_PLAY:
        {
            match->round_timer -= delta_time;

            match->f1_hp_per = ((float)p1->fighter.hp / (float)match->f1_full_hp);
            match->f2_hp_per = ((float)p2->fighter.hp / (float)match->f2_full_hp);

            // Game ends on hp or time loss
            if (match->round_timer <= 0.0f ||
                p1->fighter.hp <= 0 || p2->fighter.hp <= 0)
            {
                p1->fighter.hp = (p1->fighter.hp < 0) ? 0 : p1->fighter.hp; 
                p2->fighter.hp = (p2->fighter.hp < 0) ? 0 : p2->fighter.hp; 
                
                if (match->f1_hp_per > match->f2_hp_per)      
                { 
                    match->score_p1++;
                    match->rounds--;

                    fighter_set_state(&p2->fighter, STATE_KNOCKDOWN);
                    p2->fighter.active_stun_duration = TICKS(120);
                }
                else if (match->f1_hp_per < match->f2_hp_per) 
                { 
                    match->score_p2++;
                    match->rounds--;

                    fighter_set_state(&p1->fighter, STATE_KNOCKDOWN);
                    p1->fighter.active_stun_duration = TICKS(120);
                }
                    
                match_set_state(match, MATCH_STATE_END);
            }
            
            match_enforce_rules(p1, p2, true, delta_time);

            fighter_update_attack(&p1->fighter, &p2->fighter, match);
            fighter_update_attack(&p2->fighter, &p1->fighter, match);
    
            fighter_update(p1, &p1->fighter, delta_time);
            fighter_update(p2, &p2->fighter, delta_time);

            break;
        }
        case MATCH_STATE_START:
        {
            if (match->state_timer >= 2.0f)
            {
                match->round_timer = match->round_duration;
                p1->fighter.hp = match->f1_full_hp;    
                p2->fighter.hp = match->f2_full_hp;
                
                match_set_state(match, MATCH_STATE_PLAY);
                
                if (match->rounds <= 0) 
                {
                    if (match->score_p1 > match->score_p2) 
                    {
                        match_set_state(match, MATCH_STATE_VICTORY_P1);
                    }

                    else if (match->score_p1 < match->score_p2) 
                    {
                        match_set_state(match, MATCH_STATE_VICTORY_P2);
                    }    
                }  
            }
            
            match_enforce_rules(p1, p2, true, delta_time);
            
            fighter_update(p1, &p1->fighter, delta_time);
            fighter_update(p2, &p2->fighter, delta_time);
            
            fighter_projectile_update(&p1->fighter, &p2->fighter, delta_time);
            fighter_projectile_update(&p2->fighter, &p1->fighter, delta_time);
            
            break;
        }

        case MATCH_STATE_END:
        {
            if (match->state_timer >= 3.0f)
            {
                match_set_state(match, MATCH_STATE_START);
            }
            
            match_enforce_rules(p1, p2, true, delta_time);

            fighter_update(p1, &p1->fighter, delta_time / 2);
            fighter_update(p2, &p2->fighter, delta_time / 2);
            fighter_projectile_update(&p1->fighter, &p2->fighter, delta_time);
            fighter_projectile_update(&p2->fighter, &p1->fighter, delta_time);
            
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
                    fighter_set_state(&p2->fighter, STATE_KNOCKDOWN);
                }    
                else
                {
                    fighter_set_state(&p2->fighter, STATE_POSE_VICTORY);
                    fighter_set_state(&p1->fighter, STATE_KNOCKDOWN);
                }
            }
                
            match_enforce_rules(p1, p2, false, delta_time);
            
            fighter_update(p1, &p1->fighter, delta_time);
            fighter_update(p2, &p2->fighter, delta_time);
            fighter_projectile_update(&p1->fighter, &p2->fighter, delta_time);
            fighter_projectile_update(&p2->fighter, &p1->fighter, delta_time);
            
            break;
        }

        case MATCH_STATE_EXIT: break;
    }
} 

static void draw_dbg_boxes(renderer_t *r, fighter_t *f, SDL_Color c);
static void draw_ui_overlay(const match_t *match, renderer_t *renderer);

void match_render(const match_t *match, renderer_t *renderer)
{
    player_t *p1 = match->p1; 
    player_t *p2 = match->p2; 

    draw_ui_overlay(match, renderer);

    // Maybe a dynamic string buffer for rendering text
    static char buff[128] = "";
    static char nih[8] = "";

    // --- BAR -----
    snprintf(nih, sizeof(nih), "%02d", (int32_t)match->round_timer);
    //renderer_draw_text(renderer, LAYER_UI1, nih, (SCREEN_WIDTH / 2) - 18 + 1, 40 + 1, 20, 20, COLOR_BLACK);
    renderer_draw_text(renderer, LAYER_UI1, nih, (SCREEN_WIDTH / 2) - 18, 40, 20, 20, COLOR_WHITE);

    #define X(name) (p1->fighter.state == STATE_##name) ? #name :  
        
    snprintf(buff, sizeof(buff), 
        "\n\n\nRB,P1:%d           RB,P2:%d\n%s\nATK_ID:%d\nATK_DUR:%.1f FRAM:%d", 
        p1->fighter.ragebait_meter, 
        p2->fighter.ragebait_meter,
        FIGHTER_STATE_NAMES_XLIST "None",
        p1->fighter.curr_attack_id,
        (double)p1->fighter.state_duration,
        p1->fighter.animation_frame
    );
    #undef X
    
    renderer_draw_text(renderer, LAYER_UI1, (const char *)buff, 21, 21, 20, 20, COLOR_BLACK);
    renderer_draw_text(renderer, LAYER_UI1, (const char *)buff, 20, 20, 20, 20, COLOR_WHITE);

    draw_dbg_boxes(renderer, &p1->fighter, COLOR_RED); 
    draw_dbg_boxes(renderer, &p2->fighter, COLOR_BLUE);
    
    if (p2->fighter.curr_attack_id != ATK_ID_NONE) {
        renderer_draw_fighter(renderer, &p1->fighter);
        renderer_draw_fighter(renderer, &p2->fighter);
    } else
    {
        renderer_draw_fighter(renderer, &p2->fighter);
        renderer_draw_fighter(renderer, &p1->fighter);
    }
    // projectile rendereing
    for_range_i(lenghtof(p1->fighter.projectiles))
    {
        if (p1->fighter.projectiles[i].active)
            renderer_draw_projectile(renderer, p1->fighter.texture, &p1->fighter.projectiles[i]);
        
        if (p2->fighter.projectiles[i].active)
            renderer_draw_projectile(renderer, p2->fighter.texture, &p2->fighter.projectiles[i]);
    }
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
    
    float offset1 = match->f1_hp_per < 0.0f ? 0.0f : match->f1_hp_per;
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
    
    float offset2 = match->f2_hp_per < 0.0f ? 0.0f : match->f2_hp_per;
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
    
    renderer_draw_texture(renderer, LAYER_BACKGROUND, match->textures.stage, NULL, NULL, 0.0, SDL_FLIP_NONE);
    //tile_from_atlas(GEAR, 96, 96, 4);
    //tile_from_atlas(BAR, 96, 96, 4);

}

static void draw_dbg_boxes(renderer_t *r, fighter_t *f, SDL_Color c)
{
    const anim_frame_t *col_f = fighter_get_frame_data(f);

    for_range_j(col_f->count_hurtboxs)
    {
        SDL_Rect hurt = to_world_rect(f, col_f->hurtboxs[j]);
        renderer_draw_rect(r, LAYER_UI1, &hurt, COLOR_GREEN, false);
    }
    for_range_i(col_f->count_hitboxs)
    {
        SDL_Rect hit = to_world_rect(f, col_f->hitboxs[i]);
        renderer_draw_rect(r, LAYER_UI1, &hit, COLOR_RED, false);
    }
    SDL_Point p1 = {(int32_t)f->position.x, (int32_t)f->position.y}, 
              p2 = {(int32_t)(f->position.x + (f->velocity.x * 0.16f)),
                    (int32_t)(f->position.y + (f->velocity.y * 0.16f))};

    renderer_draw_line(r, LAYER_UI1, p1, p2, c);
}

#endif // MATCH_IMPLEMENTATION

#endif // !_MATCH_H
