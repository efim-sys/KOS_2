#include "fb_gfx.h"

/* Копирование спрайта на холст с отсечением по границам */
void draw_sprite(Canvas *canvas, const Canvas *sprite, int x, int y) {
    if (!canvas || !sprite || !canvas->data || !sprite->data) return;

    int start_x = x < 0 ? 0 : x;
    int start_y = y < 0 ? 0 : y;
    int end_x = x + sprite->width;
    int end_y = y + sprite->height;
    if (end_x > canvas->width) end_x = canvas->width;
    if (end_y > canvas->height) end_y = canvas->height;

    for (int cy = start_y; cy < end_y; cy++) {
        for (int cx = start_x; cx < end_x; cx++) {
            int sx = cx - x;
            int sy = cy - y;
            uint16_t color = sprite->data[sy * sprite->width + sx];
            canvas->data[cy * canvas->width + cx] = color;
        }
    }
}

/* Поворот на 90° по часовой стрелке */
Canvas* rotate_sprite_90(const Canvas *sprite) {
    if (!sprite) return NULL;
    Canvas *rot = create_canvas(sprite->height, sprite->width);
    if (!rot) return NULL;

    for (int y = 0; y < sprite->height; y++) {
        for (int x = 0; x < sprite->width; x++) {
            uint16_t p = sprite->data[y * sprite->width + x];
            int new_x = sprite->height - 1 - y;
            int new_y = x;
            rot->data[new_y * rot->width + new_x] = p;
        }
    }
    return rot;
}

/* Поворот на 180° */
Canvas* rotate_sprite_180(const Canvas *sprite) {
    if (!sprite) return NULL;
    Canvas *rot = create_canvas(sprite->width, sprite->height);
    if (!rot) return NULL;

    for (int y = 0; y < sprite->height; y++) {
        for (int x = 0; x < sprite->width; x++) {
            uint16_t p = sprite->data[y * sprite->width + x];
            int new_x = sprite->width - 1 - x;
            int new_y = sprite->height - 1 - y;
            rot->data[new_y * rot->width + new_x] = p;
        }
    }
    return rot;
}

/* Поворот на 270° по часовой (или 90° против часовой) */
Canvas* rotate_sprite_270(const Canvas *sprite) {
    if (!sprite) return NULL;
    Canvas *rot = create_canvas(sprite->height, sprite->width);
    if (!rot) return NULL;

    for (int y = 0; y < sprite->height; y++) {
        for (int x = 0; x < sprite->width; x++) {
            uint16_t p = sprite->data[y * sprite->width + x];
            int new_x = y;
            int new_y = sprite->width - 1 - x;
            rot->data[new_y * rot->width + new_x] = p;
        }
    }
    return rot;
}