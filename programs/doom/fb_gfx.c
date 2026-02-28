#include "fb_gfx.h"

#include "export.h"
extern struct exp_os *os;

/* Создание холста с выделением памяти */
Canvas* create_canvas(int width, int height) {
    if (width <= 0 || height <= 0) return NULL;
    Canvas *canvas = (Canvas*)os -> malloc(sizeof(Canvas));
    if (!canvas) return NULL;
    canvas->data = (uint16_t*)os -> calloc(width * height, sizeof(uint16_t));
    if (!canvas->data) {
        os->free(canvas);
        return NULL;
    }
    canvas->width = width;
    canvas->height = height;
    return canvas;
}

/* Освобождение холста */
void free_canvas(Canvas *canvas) {
    if (canvas) {
        os->free(canvas->data);
        os->free(canvas);
    }
}

/* Установка пикселя с проверкой границ */
void set_pixel(Canvas *canvas, int x, int y, uint16_t color) {
    if (!canvas || !canvas->data) return;
    if (x >= 0 && x < canvas->width && y >= 0 && y < canvas->height) {
        canvas->data[y * canvas->width + x] = color;
    }
}

/* Чтение пикселя (возвращает 0 при выходе за границы) */
uint16_t get_pixel(Canvas *canvas, int x, int y) {
    if (!canvas || !canvas->data) return 0;
    if (x >= 0 && x < canvas->width && y >= 0 && y < canvas->height) {
        return canvas->data[y * canvas->width + x];
    }
    return 0;
}

/* Алгоритм Брезенхема для линии */
void draw_line(Canvas *canvas, int x1, int y1, int x2, int y2, uint16_t color) {
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        set_pixel(canvas, x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

/* Контур прямоугольника */
void draw_rect(Canvas *canvas, int x, int y, int w, int h, uint16_t color) {
    if (w <= 0 || h <= 0) return;
    draw_line(canvas, x, y, x + w - 1, y, color);
    draw_line(canvas, x + w - 1, y, x + w - 1, y + h - 1, color);
    draw_line(canvas, x + w - 1, y + h - 1, x, y + h - 1, color);
    draw_line(canvas, x, y + h - 1, x, y, color);
}

/* Залитый прямоугольник */
void fill_rect(Canvas *canvas, int x, int y, int w, int h, uint16_t color) {
    if (w <= 0 || h <= 0) return;
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            set_pixel(canvas, x + i, y + j, color);
        }
    }
}

/* Алгоритм Брезенхема для окружности (контур) */
void draw_circle(Canvas *canvas, int x0, int y0, int radius, uint16_t color) {
    int x = radius, y = 0;
    int err = 0;

    while (x >= y) {
        set_pixel(canvas, x0 + x, y0 + y, color);
        set_pixel(canvas, x0 + y, y0 + x, color);
        set_pixel(canvas, x0 - y, y0 + x, color);
        set_pixel(canvas, x0 - x, y0 + y, color);
        set_pixel(canvas, x0 - x, y0 - y, color);
        set_pixel(canvas, x0 - y, y0 - x, color);
        set_pixel(canvas, x0 + y, y0 - x, color);
        set_pixel(canvas, x0 + x, y0 - y, color);

        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

/* Залитая окружность (горизонтальными линиями) */
void fill_circle(Canvas *canvas, int x0, int y0, int radius, uint16_t color) {
    for (int y = -radius; y <= radius; y++) {
        int h = (int)(radius * radius - y * y);
        if (h < 0) continue;
        int w = (int)sqrt(h);
        draw_line(canvas, x0 - w, y0 + y, x0 + w, y0 + y, color);
    }
}

/* Упаковка RGB в RGB565 */
uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void fill(Canvas *canvas, uint16_t color) {
    for(int i = 0; i < canvas->height * canvas->width; i++) {
        canvas->data[i] = color;
    }
}