#pragma once

#include <stdint.h>
#include <elf.h>
#include <stdio.h>
#include "defines.h"

typedef struct {
    uint8_t *data;           // Указатель на данные секции в памяти (выделено malloc)
    size_t size;             // Размер секции
    Elf32_Shdr header;       // Заголовок секции из ELF
    char *name;              // Имя секции
    uint8_t *alloc_addr;     // Адрес в ОЗУ, куда загружена секция (результат malloc)
    int is_loaded;           // Флаг: загружена ли секция в память
} SectionBuffer;

typedef struct {
    char *name;              // Имя символа
    uint32_t value;          // Смещение в секции (относительный адрес)
    uint32_t size;           // Размер символа
    uint8_t type;            // Тип символа
    uint8_t bind;            // Binding символа
    uint16_t shndx;          // Индекс секции, в которой определен символ
    uint8_t *absolute_addr;  // Абсолютный адрес символа в памяти (после загрузки)
} SymbolInfo;

typedef struct {
    FILE *file;              // Файловый дескриптор ELF файла
    Elf32_Ehdr elf_header;   // ELF заголовок
    Elf32_Shdr *section_headers;  // Таблица заголовков секций
    char *section_names;     // Строковая таблица имен секций
    SectionBuffer sections[MAX_SECTIONS];  // Буферы секций
    SymbolInfo symbols[MAX_SYMBOLS];       // Таблица символов
    int section_count;       // Количество загруженных секций
    int symbol_count;        // Количество загруженных символов
    uint32_t entry_point;    // Точка входа (адрес в памяти)
} ELFContext;
