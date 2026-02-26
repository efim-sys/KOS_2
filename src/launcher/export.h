#pragma once

#include <stdint.h>
#include <stdlib.h>

struct exp_os {
    void (*putc)(char);
    void (*putd)(int);

    void (*vTaskDelay)(uint32_t);

    uint16_t* fb;

    void (*display)(void);

    int (*printf)(const char *format, ...);

    void (*onKeyPress)(uint8_t key, void (*function)(uint8_t keyID));

    void (*digitalWrite)(uint8_t pin, uint8_t val);
    int (*digitalRead)(uint8_t pin);
    void (*pinMode)(uint8_t pin, uint8_t mode);

    void* (*malloc)(size_t size);
    void* (*calloc)(size_t nmemb, size_t size);

    void (*free)(void* data);
};