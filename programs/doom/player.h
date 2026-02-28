#pragma once
#include <stdint.h>

typedef struct _Player_t {
    float x, y;
    int8_t vn, vb, va;

    uint8_t d_angle;  // Digital angle 
    uint16_t color;

    struct _Player_t *next;
} Player_t;

Player_t* create_player(float x, float y, uint8_t d_angle, uint16_t color);
void add_player(Player_t **players, float x, float y, uint8_t d_angle, uint16_t color);
void free_players(Player_t *players);