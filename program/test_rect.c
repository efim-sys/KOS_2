asm ("j main");

#include "../src/launcher/export.h"
#include "../src/launcher/export_defines.h"

uint16_t ntohs(uint16_t netshort) {
    return ((netshort & 0xFF) << 8) | ((netshort & 0xFF00) >> 8);
}

int w = 240;
int h = 280;

void fillScreen(uint16_t* fb, uint16_t color) {
    for(int pixel = 0; pixel < w*h; pixel++) {
        fb[pixel] = color;
    }
}

/**
 * Преобразует HSV (8-бит на канал) в RGB565 (uint16_t).
 * h: 0-255 (соответствует 0-360 градусам)
 * s: 0-255 (насыщенность)
 * v: 0-255 (яркость)
 */
uint16_t hsv_to_rgb565(uint8_t h, uint8_t s, uint8_t v) {
    uint8_t r, g, b;
    uint8_t region, remainder, p, q, t;

    if (s == 0) {
        // Если насыщенность 0, то цвет — это оттенок серого
        r = g = b = v;
    } else {
        // Определяем сектор цветового круга (всего 6 секторов)
        region = h / 43; // 256 / 6 ~= 42.6
        remainder = (h - (region * 43)) * 6; // Остаток для интерполяции

        p = (v * (255 - s)) >> 8;
        q = (v * (255 - ((s * remainder) >> 8))) >> 8;
        t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

        switch (region) {
            case 0:  r = v; g = t; b = p; break;
            case 1:  r = q; g = v; b = p; break;
            case 2:  r = p; g = v; b = t; break;
            case 3:  r = p; g = q; b = v; break;
            case 4:  r = t; g = p; b = v; break;
            default: r = v; g = p; b = q; break;
        }
    }

    // Упаковка в формат RGB565:
    // R (5 бит), G (6 бит), B (5 бит)
    return ntohs(((uint16_t)(r >> 3) << 11) | 
           ((uint16_t)(g >> 2) << 5)  | 
            (uint16_t)(b >> 3));
}

void drawRect(uint16_t* fb, int cx, int cy, uint16_t color) {
    for(int y = cy-10; y < cy+10; y++) {
        for(int x = cx-10; x < cx+10; x++) {
            fb[y*w + x] = color;
        }
    }
}

struct Object {
    float x;
    float y;

    float vx; 
    float vy;
};

struct Object rect = {30, 30, 1, 1};

int8_t dv = 1;

void faster(uint8_t k) {
    dv = 1;
}

void slower(uint8_t k) {
    dv = -1;
}

int main(struct exp_os* os) {
    int array[] = {0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0xdeadbeef};
    os->printf("Printing all array now:\n");
    for(int i = 0; i < 4; i++) {
        os->printf("array[%d] = %p\n", i, array[i]);
    }
    
    os->printf("Hello, world! 41=%d\n\n\n", 41);
    os->printf("Hello, world! 41+41=42=%d\n\n\n", 42);

    os->printf("It %s works!!!!\n", "ABSOLUTELY");

    os->printf("It can do multiple: d=%d f=%f s=%s\n", 45, 3.1415f, "<- it was PI");

    fillScreen(os->fb, 0);
    os->display();

    os->printf("let's test float addition: 3.14+1.27=%f\n", 3.14f+1.27f);

    os->printf("hsv 255, 255, 255 => rgb565 %04hx\n", hsv_to_rgb565(255, 255, 255));

    os->onKeyPress(BTN_UP, faster);
    os->onKeyPress(BTN_DOWN, slower);

    uint8_t hue = 0;

    while(1) {
        fillScreen(os->fb, 0);
        drawRect(os->fb, rect.x, rect.y, hsv_to_rgb565(hue, 255, 255));

        os->display();

        rect.x += rect.vx;
        rect.y += rect.vy;

        if(rect.x < 10 || rect.x > w-10) rect.vx *= -1;
        if(rect.y < 10 || rect.y > h-10) rect.vy *= -1;

        hue ++;

        os->vTaskDelay(5);

        if(dv > 0) {
            rect.vx *= 2;
            rect.vy *= 2;
            dv = 0;
        }
        else if(dv < 0) {
            rect.vx *= 0.5;
            rect.vy *= 0.5;
            dv = 0;
        }

    }

    return 11;
}