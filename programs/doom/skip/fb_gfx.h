#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "fb_colors.h"
#include <math.h>

/* Структура холста (canvas) */
typedef struct {
    uint16_t *data;      /* массив пикселей в формате RGB565 */
    int width;           /* ширина в пикселях */
    int height;          /* высота в пикселях */
} Canvas;

/* Создание и уничтожение холста */
Canvas* create_canvas(int width, int height);
void free_canvas(Canvas *canvas);

/* Базовые операции с пикселями */
void set_pixel(Canvas *canvas, int x, int y, uint16_t color);
uint16_t get_pixel(Canvas *canvas, int x, int y);

/* Рисование примитивов */
void draw_line(Canvas *canvas, int x1, int y1, int x2, int y2, uint16_t color);
void draw_rect(Canvas *canvas, int x, int y, int w, int h, uint16_t color);
void fill_rect(Canvas *canvas, int x, int y, int w, int h, uint16_t color);
void draw_circle(Canvas *canvas, int x0, int y0, int radius, uint16_t color);
void fill_circle(Canvas *canvas, int x0, int y0, int radius, uint16_t color);

/* Работа со спрайтами (спрайт – тоже холст) */
void draw_sprite(Canvas *canvas, const Canvas *sprite, int x, int y);
Canvas* rotate_sprite_90(const Canvas *sprite);   /* по часовой стрелке */
Canvas* rotate_sprite_180(const Canvas *sprite);
Canvas* rotate_sprite_270(const Canvas *sprite); /* против часовой (или 270° по часовой) */

/* Вспомогательная функция для упаковки RGB в RGB565 */
uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b);