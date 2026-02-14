#pragma once

#include <stdint.h>

enum {
  BTN_UP = 0,
  BTN_DOWN,
  BTN_BOOT,
  JOY_UP,
  JOY_DOWN,
  JOY_LEFT,
  JOY_RIGHT,
  JOY_CENTER
};

struct exp_os {
    void (*putc)(char);
    void (*putd)(int);

    void (*vTaskDelay)(uint32_t);

    uint16_t* fb;

    void (*display)(void);

    int (*printf)(const char *format, ...);

    void (*onKeyPress)(uint8_t key, void (*function)(uint8_t keyID));
};