#include "player.h"
#include "export.h"

extern struct exp_os *os;

Player_t* create_player(float x, float y, uint8_t d_angle, uint16_t color) {
    Player_t *player = os->malloc(sizeof(Player_t));

    *player = (Player_t) {
        .x = x,
        .y = y,
        
        .vn = 0,
        .vb = 0,
        .va = 0,
        

        .d_angle = d_angle,
        .color = color
    };

    return player;
}

void add_player(Player_t **players, float x, float y, uint8_t d_angle, uint16_t color) {
    Player_t *new_player = create_player(x, y, d_angle, color);

    if(*players == NULL) *players = new_player;
    else {
        Player_t* last_player = *players;

        while(last_player -> next) last_player = last_player->next;

        last_player->next = new_player;
    }
}

void free_players(Player_t *players) {

    Player_t *current_player = players;
    Player_t *next_player;

    while(current_player) {
        next_player = current_player -> next;

        os->free(current_player);

        current_player = next_player;
    }
}