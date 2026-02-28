#include <math.h>
#include "export.h"
#include "export_defines.h"
#include "fb_gfx.h"

#include "player.h"

#define LEVEL_W 32
#define LEVEL_H 32

#define cell(lvl, x, y) lvl[x + y * LEVEL_W]

#define RECT_SIZE_2D 32

#undef M_PI
#define M_PI 3.14159265359f 
#define sign(f) ((f > 0) ? 1 : -1)

float norm_speed = 0.1f;
uint8_t d_turn_speed = 1;

#define d_angle_max 32 // 32 rotation positions

#define f_angle(d) (d * 2.0f * M_PI / d_angle_max)

struct exp_os *os;
Canvas canvas;


typedef struct {
    float x, y;
} Intersection;




enum Wall_id_t {
    WALL_EMPTY = 0,
    BRICK_WALL = 10,
    PORTRAIT_WALL = 11
};

void castRay(Intersection *intersection, float x0, float y0, float nx, float ny);

void draw_grid_2D(const uint8_t* level) {
    for(int y = 0; y < LEVEL_H; y ++) {
        for(int x = 0; x < LEVEL_W; x++) {

            uint8_t wall_id = cell(level, x, y);

            switch(wall_id) {
                case WALL_EMPTY:
                    draw_rect(&canvas, RECT_SIZE_2D*x, RECT_SIZE_2D*y, RECT_SIZE_2D, RECT_SIZE_2D, COLOR_GRAY);
                    break;
                case BRICK_WALL:
                    fill_rect(&canvas, RECT_SIZE_2D*x, RECT_SIZE_2D*y, RECT_SIZE_2D, RECT_SIZE_2D, COLOR_GRAY);
                    break;
                default:
                    draw_rect(&canvas, RECT_SIZE_2D*x, RECT_SIZE_2D*y, RECT_SIZE_2D, RECT_SIZE_2D, COLOR_RED);
                    break;
            }

        }
    }
}

void draw_player_2D(const Player_t *player) {
    int display_x = player->x*RECT_SIZE_2D;
    int display_y = player->y*RECT_SIZE_2D;

    fill_circle(&canvas, display_x, display_y, 5, player->color);

    // draw_line(&canvas, display_x, display_y, 
    //     display_x + cosf(f_angle(player->d_angle)) * 20,
    //     display_y + sinf(f_angle(player->d_angle)) * 20,
    //     player->color);

    Intersection intersection;

    castRay(&intersection, player->x, player->y, 
        cosf(f_angle(player->d_angle)),             // Normal x
        sinf(f_angle(player->d_angle))              // Normal y
    );

    draw_line(&canvas, display_x, display_y, 
        display_x + intersection.x * 20,
        display_y + intersection.y * 20,
        player->color);
}

void render_2D(const uint8_t *level, const Player_t *players) {
    fill(&canvas, COLOR_BLACK);

    draw_grid_2D(level);

    const Player_t *next_player = players;

    while(next_player) {
        draw_player_2D(next_player);
        next_player = next_player -> next;
    }
}

void set_speed_btn(Player_t *player) {
    if(os->digitalRead(os->pin_num[JOY_UP])) player->vn = 1;
    else if(os->digitalRead(os->pin_num[JOY_DOWN])) player->vn = -1;

    if(os->digitalRead(os->pin_num[JOY_LEFT])) {
        if(os->digitalRead(os->pin_num[JOY_CENTER])) player->vb = 1;  // LEFT+CENTER = move binormally left
        else player->va = 1;    // LEFT = rotate ccv
    }
    else if(os->digitalRead(os->pin_num[JOY_RIGHT])) {
        if(os->digitalRead(os->pin_num[JOY_CENTER])) player->vb = -1;  // RIGHT+CENTER = move binormally right
        else player->va = -1;
    }
}

void add_velocity(Player_t *player, float angle_add) {
    float vx = cosf(f_angle(player->d_angle)  + angle_add) * norm_speed;
    float vy = sinf(f_angle(player->d_angle) + angle_add) * norm_speed;

    player->x += vx;
    player->y += vy;
}

void apply_speed(Player_t * players) {
    while(players) {

        if (players->vn != 0) add_velocity(players, ((players->vn > 0) ? 0.0f : M_PI)); // add normal velocity
        if (players->vb != 0) add_velocity(players, ((players->vb < 0) ? M_PI/2.0f : -M_PI/2.0f)); // add binormal velocity

        if      (players->va > 0) players->d_angle -= d_turn_speed;
        else if (players->va < 0) players->d_angle += d_turn_speed;

        players->d_angle = players->d_angle % d_angle_max;

        players -> vn = 0;
        players -> va = 0;
        players -> vb = 0;
        
        players = players -> next;
    }
}

void castRay(Intersection *intersection, float x0, float y0, float nx, float ny) {
    float side_dst_x, side_dst_y;

    side_dst_x = ((sign(nx) == 1) ? ceilf(nx) : floorf(nx)) - x0;
    side_dst_y = ((sign(ny) == 1) ? ceilf(ny) : floorf(ny)) - y0;

    intersection->x = x0 + side_dst_x;
    intersection->y = y0 + side_dst_y;
}




int main(struct exp_os* _os) {
    os = _os;

    canvas = (Canvas) {
        .data = os->fb,
        .width = 240,
        .height = 280
    };


    uint8_t level[LEVEL_H*LEVEL_W] = {0};

    cell(level, 3, 4) = BRICK_WALL;
    cell(level, 3, 5) = BRICK_WALL;
    cell(level, 3, 6) = BRICK_WALL;

    Player_t* players = NULL;
    add_player(&players, 2.5f, 3.5f, 0.0f, COLOR_GREEN);
    add_player(&players, 4.5f, 5.5f, 0.0f, COLOR_RED);
    add_player(&players, 5.5f, 5.5f, 0.0f, COLOR_CYAN);
    add_player(&players, 3.5f, 5.5f, 0.0f, COLOR_NAVY);

    os->printf("players = %p\n", players);

    Player_t *main_player = players;

    while(1) {
        set_speed_btn(main_player);
        apply_speed(players);

        os->printf("Main_player: x=%f y=%f angle=%f\n", main_player->x, main_player->y, f_angle(main_player->d_angle));

        render_2D(level, players);

        os->display();
    }

    

    free_players(players);
    
    return 52;
}