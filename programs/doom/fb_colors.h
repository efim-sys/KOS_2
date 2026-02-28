#pragma once

#define ntohs(n) ((((uint16_t)(n) & 0xff00) >> 8) | \
                  (((uint16_t)(n) & 0x00ff) << 8))

/* Макрос для формирования цвета RGB565 из трёх 8-битных компонентов */
#define RGB565(r, g, b)  \
    ntohs((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

/* Предопределённые цвета (значения по умолчанию) */
#define COLOR_BLACK       RGB565(0, 0, 0)
#define COLOR_WHITE       RGB565(255, 255, 255)
#define COLOR_RED         RGB565(255, 0, 0)
#define COLOR_GREEN       RGB565(0, 255, 0)
#define COLOR_BLUE        RGB565(0, 0, 255)
#define COLOR_YELLOW      RGB565(255, 255, 0)
#define COLOR_CYAN        RGB565(0, 255, 255)
#define COLOR_MAGENTA     RGB565(255, 0, 255)
#define COLOR_GRAY        RGB565(128, 128, 128)
#define COLOR_DARK_RED    RGB565(128, 0, 0)
#define COLOR_DARK_GREEN  RGB565(0, 128, 0)
#define COLOR_DARK_BLUE   RGB565(0, 0, 128)
#define COLOR_ORANGE      RGB565(255, 165, 0)
#define COLOR_PURPLE      RGB565(128, 0, 128)
#define COLOR_PINK        RGB565(255, 192, 203)
#define COLOR_BROWN       RGB565(165, 42, 42)
#define COLOR_LIME        RGB565(0, 255, 0)      /* совпадает с GREEN, оставлено для удобства */
#define COLOR_TEAL        RGB565(0, 128, 128)
#define COLOR_NAVY        RGB565(0, 0, 128)      /* совпадает с DARK_BLUE */
#define COLOR_MAROON      RGB565(128, 0, 0)      /* совпадает с DARK_RED */
#define COLOR_OLIVE       RGB565(128, 128, 0)