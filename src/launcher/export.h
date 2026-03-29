#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

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

    uint8_t pin_num[24];

    FILE* (*fopen)(const char* filename, const char* mode);

    void* (*ps_malloc)(size_t size);

    int	(*fseek) (FILE *, long, int);
    size_t	(*fread) (void *__restrict, size_t _size, size_t _n, FILE *__restrict);
    int	(*fclose) (FILE *);

    long	(*ftell) ( FILE *);

    uint32_t (*rand)(void);

    size_t	(*fwrite) (const void *__restrict , size_t _size, size_t _n, FILE *);
};