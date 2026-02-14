#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <elf.h>

// Определения для Xtensa (ESP32)
#define R_XTENSA_NONE           0
#define R_XTENSA_32             1
#define R_XTENSA_32_PCREL       3
#define R_XTENSA_32_ASM_EXPAND  11
#define R_XTENSA_SLOT0_OP       20
#define R_XTENSA_SLOT1_OP       21
#define R_XTENSA_SLOT2_OP       22
#define R_XTENSA_SLOT3_OP       23
#define R_XTENSA_SLOT4_OP       24
#define R_XTENSA_SLOT5_OP       25
#define R_XTENSA_SLOT6_OP       26
#define R_XTENSA_SLOT7_OP       27
#define R_XTENSA_SLOT8_OP       28
#define R_XTENSA_SLOT9_OP       29
#define R_XTENSA_SLOT10_OP      30
#define R_XTENSA_SLOT11_OP      31
#define R_XTENSA_SLOT12_OP      32
#define R_XTENSA_SLOT13_OP      33
#define R_XTENSA_SLOT14_OP      34
#define R_XTENSA_OP0           35
#define R_XTENSA_OP1           36
#define R_XTENSA_OP2           37
#define R_XTENSA_DIFF8         51
#define R_XTENSA_DIFF16        52
#define R_XTENSA_DIFF32        53
#define R_XTENSA_PDIFF8        54
#define R_XTENSA_PDIFF16       55
#define R_XTENSA_PDIFF32       56
#define R_XTENSA_GLOB_DAT      100
#define R_XTENSA_JMP_SLOT      101
#define R_XTENSA_RELATIVE      102

#define MAX_SECTIONS 50
#define MAX_SYMBOLS 1000

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

// Прототипы функций
int parse_elf(const char *filename, ELFContext *ctx);
int load_sections_to_memory(ELFContext *ctx);
void apply_relocations(ELFContext *ctx);
void process_relocation(Elf32_Rela *rela, SectionBuffer *target, 
                       SectionBuffer *reloc_section, ELFContext *ctx, int reloc_index);
void process_rel(Elf32_Rel *rel, SectionBuffer *target, 
                SectionBuffer *reloc_section, ELFContext *ctx, int reloc_index);
uint8_t* get_symbol_absolute_addr(ELFContext *ctx, uint32_t sym_index);
char* get_symbol_name(ELFContext *ctx, uint32_t sym_index);
uint32_t get_symbol_section(ELFContext *ctx, uint32_t sym_index);
void cleanup(ELFContext *ctx);
void print_section_info(ELFContext *ctx);
void decode_xtensa_instruction(uint32_t instr, uint32_t *op0, uint32_t *op1, uint32_t *op2);
int load_symbols(ELFContext *ctx);
int calculate_symbol_addresses(ELFContext *ctx);
const char* get_relocation_type_name(uint32_t type);
const char* get_section_type_name(uint32_t type);
int should_load_section(Elf32_Shdr *shdr);
int is_relocation_section(Elf32_Shdr *shdr);
void print_separator(int length);
int is_metadata_section(const char *name, Elf32_Shdr *shdr);
int find_section_by_name(ELFContext *ctx, const char *name);
SectionBuffer* find_section_by_index(ELFContext *ctx, int index);

// Вспомогательная функция для печати разделителя
void print_separator(int length) {
    for (int i = 0; i < length; i++) {
        printf("-");
    }
    printf("\n");
}

const char* get_section_type_name(uint32_t type) {
    switch(type) {
        case SHT_NULL: return "NULL";
        case SHT_PROGBITS: return "PROGBITS";
        case SHT_SYMTAB: return "SYMTAB";
        case SHT_STRTAB: return "STRTAB";
        case SHT_RELA: return "RELA";
        case SHT_HASH: return "HASH";
        case SHT_DYNAMIC: return "DYNAMIC";
        case SHT_NOTE: return "NOTE";
        case SHT_NOBITS: return "NOBITS";
        case SHT_REL: return "REL";
        case SHT_SHLIB: return "SHLIB";
        case SHT_DYNSYM: return "DYNSYM";
        default: return "UNKNOWN";
    }
}

int should_load_section(Elf32_Shdr *shdr) {
    // Загружаем только секции с флагом SHF_ALLOC (те, которые должны быть в памяти)
    return (shdr->sh_flags & SHF_ALLOC) != 0;
}

int is_relocation_section(Elf32_Shdr *shdr) {
    // Проверяем, является ли секция секцией релокаций
    return (shdr->sh_type == SHT_RELA || shdr->sh_type == SHT_REL);
}

int is_metadata_section(const char *name, Elf32_Shdr *shdr) {
    // Проверяем, является ли секция метаданными (не загружаемыми в память выполнения)
    if (shdr->sh_type == SHT_SYMTAB ||
        shdr->sh_type == SHT_STRTAB ||
        shdr->sh_type == SHT_RELA ||
        shdr->sh_type == SHT_REL) {
        return 1;
    }
    
    // Проверяем по имени
    if (name == NULL) return 0;
    
    if (strstr(name, ".rela") == name || 
        strstr(name, ".rel") == name ||
        strstr(name, ".symtab") ||
        strstr(name, ".strtab") ||
        strstr(name, ".shstrtab") ||
        strstr(name, ".comment") ||
        strstr(name, ".debug") ||
        strstr(name, ".note") ||
        strstr(name, ".xtensa.info")) {
        return 1;
    }
    
    return 0;
}

int find_section_by_name(ELFContext *ctx, const char *name) {
    for (int i = 0; i < ctx->section_count; i++) {
        if (strcmp(ctx->sections[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

SectionBuffer* find_section_by_index(ELFContext *ctx, int index) {
    if (index >= 0 && index < ctx->section_count) {
        return &ctx->sections[index];
    }
    return NULL;
}

int parse_elf(const char *filename, ELFContext *ctx) {
    // Открываем файл
    ctx->file = fopen(filename, "rb");
    if (!ctx->file) {
        perror("Failed to open file");
        return -1;
    }

    // Читаем ELF заголовок
    if (fread(&ctx->elf_header, sizeof(Elf32_Ehdr), 1, ctx->file) != 1) {
        fprintf(stderr, "Failed to read ELF header\n");
        fclose(ctx->file);
        return -1;
    }

    // Проверяем магические числа
    if (memcmp(ctx->elf_header.e_ident, ELFMAG, SELFMAG) != 0) {
        fprintf(stderr, "Not a valid ELF file\n");
        fclose(ctx->file);
        return -1;
    }

    // Проверяем архитектуру (должна быть Xtensa)
    if (ctx->elf_header.e_machine != 0x5E && ctx->elf_header.e_machine != 94) {
        fprintf(stderr, "Not an Xtensa/ESP32 ELF file (e_machine = 0x%x)\n", ctx->elf_header.e_machine);
        fclose(ctx->file);
        return -1;
    }

    // Сохраняем точку входа
    ctx->entry_point = ctx->elf_header.e_entry;
    
    // Переходим к таблице заголовков секций
    fseek(ctx->file, ctx->elf_header.e_shoff, SEEK_SET);
    
    // Выделяем память для заголовков секций
    ctx->section_headers = malloc(ctx->elf_header.e_shnum * sizeof(Elf32_Shdr));
    if (!ctx->section_headers) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(ctx->file);
        return -1;
    }

    // Читаем заголовки секций
    if (fread(ctx->section_headers, sizeof(Elf32_Shdr), 
              ctx->elf_header.e_shnum, ctx->file) != ctx->elf_header.e_shnum) {
        fprintf(stderr, "Failed to read section headers\n");
        free(ctx->section_headers);
        fclose(ctx->file);
        return -1;
    }

    // Получаем строковую таблицу для имен секций
    Elf32_Shdr *strtab_header = &ctx->section_headers[ctx->elf_header.e_shstrndx];
    ctx->section_names = malloc(strtab_header->sh_size);
    if (!ctx->section_names) {
        fprintf(stderr, "Memory allocation failed\n");
        free(ctx->section_headers);
        fclose(ctx->file);
        return -1;
    }

    fseek(ctx->file, strtab_header->sh_offset, SEEK_SET);
    if (fread(ctx->section_names, 1, strtab_header->sh_size, ctx->file) != strtab_header->sh_size) {
        fprintf(stderr, "Failed to read section names\n");
        free(ctx->section_names);
        free(ctx->section_headers);
        fclose(ctx->file);
        return -1;
    }

    // Инициализируем буферы секций
    ctx->section_count = 0;
    
    // Сначала загружаем информацию о всех секциях
    for (int i = 0; i < ctx->elf_header.e_shnum && ctx->section_count < MAX_SECTIONS; i++) {
        Elf32_Shdr *shdr = &ctx->section_headers[i];
        
        // Пропускаем пустые секции
        if (shdr->sh_size == 0 || shdr->sh_type == SHT_NULL) continue;
        
        char *section_name = &ctx->section_names[shdr->sh_name];
        
        SectionBuffer *buf = &ctx->sections[ctx->section_count];
        buf->header = *shdr;
        buf->size = shdr->sh_size;
        buf->name = section_name;
        buf->data = NULL;
        buf->alloc_addr = NULL;
        buf->is_loaded = 0;
        
        ctx->section_count++;
    }

    return 0;
}

int load_sections_to_memory(ELFContext *ctx) {
    printf("[LOADER] Загрузка секций в память:\n");
    print_separator(60);
    
    int loaded_count = 0;
    
    for (int i = 0; i < ctx->section_count; i++) {
        SectionBuffer *section = &ctx->sections[i];
        
        // Проверяем, нужно ли загружать эту секцию
        if (is_metadata_section(section->name, &section->header)) {
            printf("[SKIP] Пропускаем метаданные: '%s' (тип: %s)\n", 
                   section->name, get_section_type_name(section->header.sh_type));
            continue;
        }
        
        // Проверяем, имеет ли секция флаг ALLOC
        if (!should_load_section(&section->header)) {
            printf("[SKIP] Пропускаем секцию без флага ALLOC: '%s'\n", section->name);
            continue;
        }
        
        // Выделяем память для секции
        section->data = malloc(section->size);
        if (!section->data) {
            fprintf(stderr, "Failed to allocate %zu bytes for section '%s'\n", 
                    section->size, section->name);
            continue;
        }
        
        // Для секций типа NOBITS (например, .bss) заполняем нулями
        if (section->header.sh_type == SHT_NOBITS) {
            memset(section->data, 0, section->size);
        } else {
            // Читаем данные секции из файла
            fseek(ctx->file, section->header.sh_offset, SEEK_SET);
            if (fread(section->data, 1, section->size, ctx->file) != section->size) {
                fprintf(stderr, "Failed to read data for section '%s'\n", section->name);
                free(section->data);
                section->data = NULL;
                continue;
            }
        }
        
        section->alloc_addr = section->data;
        section->is_loaded = 1;
        
        printf("[LOADED] Секция: %s\n", section->name);
        printf("         Адрес в ОЗУ: %p\n", (void*)section->alloc_addr);
        printf("         Размер: %zu байт\n", section->size);
        printf("         Тип: %s\n", get_section_type_name(section->header.sh_type));
        printf("         Флаги: 0x%08x", section->header.sh_flags);
        if (section->header.sh_flags & SHF_EXECINSTR) printf(" EXECINSTR");
        if (section->header.sh_flags & SHF_WRITE) printf(" WRITE");
        if (section->header.sh_flags & SHF_ALLOC) printf(" ALLOC");
        printf("\n\n");
        
        loaded_count++;
    }
    
    return loaded_count;
}

int load_symbols(ELFContext *ctx) {
    ctx->symbol_count = 0;
    
    // Ищем таблицу символов (.symtab)
    for (int i = 0; i < ctx->elf_header.e_shnum; i++) {
        Elf32_Shdr *shdr = &ctx->section_headers[i];
        
        if (shdr->sh_type == SHT_SYMTAB) {
            char *section_name = &ctx->section_names[shdr->sh_name];
            printf("[LOADER] Найдена таблица символов: %s (размер: %u)\n", 
                   section_name, shdr->sh_size);
            
            // Выделяем временный буфер для таблицы символов
            uint8_t *symtab_data = malloc(shdr->sh_size);
            if (!symtab_data) {
                fprintf(stderr, "Failed to allocate memory for symbol table\n");
                return -1;
            }
            
            // Читаем таблицу символов из файла
            fseek(ctx->file, shdr->sh_offset, SEEK_SET);
            if (fread(symtab_data, 1, shdr->sh_size, ctx->file) != shdr->sh_size) {
                fprintf(stderr, "Failed to read symbol table\n");
                free(symtab_data);
                return -1;
            }
            
            Elf32_Sym *symtab = (Elf32_Sym *)symtab_data;
            uint32_t num_symbols = shdr->sh_size / sizeof(Elf32_Sym);
            
            // Получаем строковую таблицу для имен символов
            Elf32_Shdr *strtab_hdr = &ctx->section_headers[shdr->sh_link];
            char *strtab = malloc(strtab_hdr->sh_size);
            if (strtab) {
                fseek(ctx->file, strtab_hdr->sh_offset, SEEK_SET);
                fread(strtab, 1, strtab_hdr->sh_size, ctx->file);
            }
            
            printf("[LOADER] Загружаем символы (%u записей):\n", num_symbols);
            
            for (uint32_t j = 0; j < num_symbols && ctx->symbol_count < MAX_SYMBOLS; j++) {
                SymbolInfo *sym = &ctx->symbols[ctx->symbol_count];
                sym->value = symtab[j].st_value;
                sym->size = symtab[j].st_size;
                sym->type = ELF32_ST_TYPE(symtab[j].st_info);
                sym->bind = ELF32_ST_BIND(symtab[j].st_info);
                sym->shndx = symtab[j].st_shndx;
                sym->absolute_addr = NULL;  // Пока не вычислен
                
                if (strtab && symtab[j].st_name > 0 && symtab[j].st_name < strtab_hdr->sh_size) {
                    sym->name = strdup(&strtab[symtab[j].st_name]);
                } else {
                    sym->name = strdup("unnamed");
                }
                
                ctx->symbol_count++;
            }
            
            if (strtab) free(strtab);
            free(symtab_data);
            
            printf("[LOADER] Загружено символов: %d\n", ctx->symbol_count);
            return 0;
        }
    }
    
    printf("[LOADER] Таблица символов не найдена\n");
    return -1;
}

int calculate_symbol_addresses(ELFContext *ctx) {
    printf("[LOADER] Вычисление абсолютных адресов символов:\n");
    print_separator(60);
    
    for (int i = 0; i < ctx->symbol_count; i++) {
        SymbolInfo *sym = &ctx->symbols[i];
        
        if (sym->shndx == SHN_UNDEF) {
            // Неопределенный символ (внешний) - адрес пока неизвестен
            sym->absolute_addr = NULL;
            printf("  %-30s: EXTERNAL (секция: UNDEF)\n", sym->name);
        } else if (sym->shndx == SHN_ABS) {
            // Абсолютный символ - значение уже является адресом
            sym->absolute_addr = (uint8_t*)(uintptr_t)sym->value;
            printf("  %-30s: ABSOLUTE (адрес: %p)\n", sym->name, (void*)sym->absolute_addr);
        } else if (sym->shndx == SHN_COMMON) {
            // COMMON символ (неинициализированные глобальные данные)
            sym->absolute_addr = NULL;
            printf("  %-30s: COMMON (размер: 0x%08x)\n", sym->name, sym->size);
        } else if (sym->shndx >= ctx->elf_header.e_shnum) {
            // Некорректный индекс секции
            sym->absolute_addr = NULL;
            printf("  %-30s: ERROR (некорректный индекс секции: %u)\n", 
                   sym->name, sym->shndx);
        } else {
            // Обычный символ: ищем его секцию
            Elf32_Shdr *section_hdr = &ctx->section_headers[sym->shndx];
            char *section_name = &ctx->section_names[section_hdr->sh_name];
            
            // Ищем секцию в нашем массиве загруженных секций
            SectionBuffer *loaded_section = NULL;
            for (int j = 0; j < ctx->section_count; j++) {
                if (ctx->sections[j].header.sh_offset == section_hdr->sh_offset) {
                    loaded_section = &ctx->sections[j];
                    break;
                }
            }
            
            if (loaded_section && loaded_section->is_loaded) {
                // Вычисляем абсолютный адрес: адрес секции в памяти + смещение в секции
                sym->absolute_addr = loaded_section->alloc_addr + sym->value;
                printf("  %-30s: %p (секция: %s, смещение: 0x%08x)\n", 
                       sym->name, (void*)sym->absolute_addr, section_name, sym->value);
            } else {
                // Секция не загружена в память
                sym->absolute_addr = NULL;
                printf("  %-30s: NOT LOADED (секция %s не загружена)\n", 
                       sym->name, section_name);
            }
        }
    }
    printf("\n");
    return 0;
}

uint8_t* get_symbol_absolute_addr(ELFContext *ctx, uint32_t sym_index) {
    if (sym_index < ctx->symbol_count) {
        return ctx->symbols[sym_index].absolute_addr;
    }
    return NULL;
}

char* get_symbol_name(ELFContext *ctx, uint32_t sym_index) {
    if (sym_index < ctx->symbol_count) {
        return ctx->symbols[sym_index].name;
    }
    return "unknown";
}

uint32_t get_symbol_section(ELFContext *ctx, uint32_t sym_index) {
    if (sym_index < ctx->symbol_count) {
        return ctx->symbols[sym_index].shndx;
    }
    return SHN_UNDEF;
}

const char* get_relocation_type_name(uint32_t type) {
    switch(type) {
        case R_XTENSA_NONE: return "R_XTENSA_NONE";
        case R_XTENSA_32: return "R_XTENSA_32";
        case R_XTENSA_32_PCREL: return "R_XTENSA_32_PCREL";
        case R_XTENSA_SLOT0_OP: return "R_XTENSA_SLOT0_OP";
        case R_XTENSA_SLOT1_OP: return "R_XTENSA_SLOT1_OP";
        case R_XTENSA_SLOT2_OP: return "R_XTENSA_SLOT2_OP";
        case R_XTENSA_SLOT3_OP: return "R_XTENSA_SLOT3_OP";
        case R_XTENSA_SLOT4_OP: return "R_XTENSA_SLOT4_OP";
        case R_XTENSA_SLOT5_OP: return "R_XTENSA_SLOT5_OP";
        case R_XTENSA_SLOT6_OP: return "R_XTENSA_SLOT6_OP";
        case R_XTENSA_SLOT7_OP: return "R_XTENSA_SLOT7_OP";
        case R_XTENSA_SLOT8_OP: return "R_XTENSA_SLOT8_OP";
        case R_XTENSA_SLOT9_OP: return "R_XTENSA_SLOT9_OP";
        case R_XTENSA_SLOT10_OP: return "R_XTENSA_SLOT10_OP";
        case R_XTENSA_SLOT11_OP: return "R_XTENSA_SLOT11_OP";
        case R_XTENSA_SLOT12_OP: return "R_XTENSA_SLOT12_OP";
        case R_XTENSA_SLOT13_OP: return "R_XTENSA_SLOT13_OP";
        case R_XTENSA_SLOT14_OP: return "R_XTENSA_SLOT14_OP";
        case R_XTENSA_OP0: return "R_XTENSA_OP0";
        case R_XTENSA_OP1: return "R_XTENSA_OP1";
        case R_XTENSA_OP2: return "R_XTENSA_OP2";
        case R_XTENSA_DIFF8: return "R_XTENSA_DIFF8";
        case R_XTENSA_DIFF16: return "R_XTENSA_DIFF16";
        case R_XTENSA_DIFF32: return "R_XTENSA_DIFF32";
        case R_XTENSA_GLOB_DAT: return "R_XTENSA_GLOB_DAT";
        case R_XTENSA_JMP_SLOT: return "R_XTENSA_JMP_SLOT";
        case R_XTENSA_RELATIVE: return "R_XTENSA_RELATIVE";
        default: {
            static char buf[32];
            snprintf(buf, sizeof(buf), "UNKNOWN_%u", type);
            return buf;
        }
    }
}

void decode_xtensa_instruction(uint32_t instr, uint32_t *op0, uint32_t *op1, uint32_t *op2) {
    *op0 = (instr >> 0) & 0xF;
    *op1 = (instr >> 4) & 0xF;
    *op2 = (instr >> 8) & 0xF;
}

// void apply_xtensa_slot0_reloc(uint8_t *code_ptr, uint32_t target_addr) {
//     uint32_t p = (uint32_t)code_ptr;
    
//     // В Xtensa (L32R) база вычисляется как (PC + 3) & ~3
//     uint32_t base = (p + 3) & ~3;
    
//     // target_addr — это обычно S + A (значение символа + адденда)
//     int32_t offset_bytes = (int32_t)(target_addr - base);
//     int32_t offset_val = offset_bytes >> 2;

//     printf("[DEBUG] RELOC R_XTENSA_SLOT0_OP\n");
//     printf("  Location (P):  0x%08X\n", p);
//     // printf("  Symbol Val (S):0x%08X\n", sym_val);
//     // printf("  Addend (A):    %d\n", addend);
//     printf("  Target (S+A):  0x%08X\n", target_addr);
//     printf("  L32R Base:     0x%08X\n", base);
    
//     // Проверка на корректность смещения
//     if (offset_bytes % 4 != 0) {
//         printf("  [ERROR] Offset 0x%X is not 4-byte aligned!\n", offset_bytes);
//     }
    
//     // L32R позволяет прыгать только назад или вперед на 256КБ
//     // Смещение 16-битное знаковое
//     if (offset_val < -32768 || offset_val > 32767) {
//         printf("  [ERROR] Relocation truncated! Offset %d is out of 16-bit range\n", offset_val);
//     }

//     // Состояние ДО
//     printf("  Instruction before: %02X %02X %02X\n", code_ptr[0], code_ptr[1], code_ptr[2]);

//     uint16_t rel_val = (uint16_t)(offset_val & 0xFFFF);
    
//     // Модификация (сохраняем opcode в code_ptr[0])
//     code_ptr[1] = (uint8_t)(rel_val & 0xFF);
//     code_ptr[2] = (uint8_t)((rel_val >> 8) & 0xFF);

//     // Состояние ПОСЛЕ
//     printf("  Instruction after:  %02X %02X %02X (Offset: %d words)\n", 
//             code_ptr[0], code_ptr[1], code_ptr[2], offset_val);
//     printf("  --------------------------------------------------\n");
// }


void apply_xtensa_slot0_reloc(uint8_t *p_ptr, uint32_t target_addr) {
    uint32_t p = (uint32_t)p_ptr;
    uint8_t opcode = p_ptr[0];
    
    printf("[DEBUG] Slot0 Reloc at 0x%08X (Opcode: 0x%02X)\n", p, opcode);

    // Состояние ДО
    printf("  Instruction before: %02X %02X %02X\n", p_ptr[0], p_ptr[1], p_ptr[2]);

    int32_t offset;

    // 1. L32R (Смещение 16 бит, значащие биты [16:31] инструкции, PC-relative)
    if (opcode == 0x81) {
        uint32_t base = (p + 3) & ~3;
        offset = (int32_t)(target_addr - base) >> 2;
        
        printf("  Type: L32R (16-bit offset: %d !words!)\n", offset);
        p_ptr[1] = (uint8_t)(offset & 0xFF);
        p_ptr[2] = (uint8_t)((offset >> 8) & 0xFF);
    }
    
    // 2. Условные переходы (BCC) - смещение 8 бит
    // Примеры: BEQ, BNE, BGEU, BLTU... (Opcode обычно заканчивается на 0x7 или 0x6 в зависимости от типа)
    else if ((opcode & 0xF) == 0x7 || (opcode & 0xF) == 0x6) {
        offset = (int32_t)(target_addr - p);
        
        printf("  Type: Branch (8-bit offset: %d)\n", offset);
        if (offset < -128 || offset > 127) {
            printf("  [ERROR] Branch target out of 8-bit range!\n");
        }
        // Смещение в этих инструкциях обычно лежит во 3-м байте (p_ptr[2])
        // p_ptr[2] = (uint8_t)(offset & 0xFF);
        printf("{SKIP for now}\n");
    }

    // 3. CALL (Смещение 18 бит, сдвинутое на 2)
    else if ((opcode & 0xF) == 0x5) {
        offset = (int32_t)(target_addr - ((p + 4) & ~3)) >> 2;
        
        printf("  Type: CALL (18-bit offset: %d)\n", offset);
        // Нужно упаковать 18 бит в 3-байтовую инструкцию
        // Биты смещения распределены между байтами 0, 1 и 2
        p_ptr[0] = (uint8_t)((opcode & 0x3F) | ((offset & 0x3) << 6));
        p_ptr[1] = (uint8_t)((offset >> 2) & 0xFF);
        p_ptr[2] = (uint8_t)((offset >> 10) & 0xFF);
    }

    // 4. Инструкции Density (16-битные, например BEQZ.N) - смещение 6 или 12 бит
    else if ((opcode & 0xF) == 0xC || (opcode & 0xF) == 0xD) {
        // Это упрощенный пример для коротких инструкций
        offset = (int32_t)(target_addr - p - 4);
        printf("  Type: Density Branch (Short offset: %d)\n", offset);
        
        // В 16-битных инструкциях смещение упаковано специфично для каждой
        // Например, для BEQZ.N (Opcode 0x8C..0xCC):
        printf("{SKIP for now}\n");
        // p_ptr[1] = (p_ptr[1] & 0xF0) | ((offset >> 2) & 0x0F); // Пример упаковки
    }

    else {
        printf("  [WARNING] Unknown Opcode for Slot0! Check Xtensa ISA.\n");
    }

    printf("  Instruction after:  %02X %02X %02X (Offset: %d bytes)\n", 
        p_ptr[0], p_ptr[1], p_ptr[2], offset);
}

void process_relocation(Elf32_Rela *rela, SectionBuffer *target, 
                       SectionBuffer *reloc_section, ELFContext *ctx, int reloc_index) {
    uint32_t offset = rela->r_offset;
    uint32_t type = ELF32_R_TYPE(rela->r_info);
    uint32_t sym_index = ELF32_R_SYM(rela->r_info);
    int32_t addend = rela->r_addend;

    
    
    // Проверяем, что смещение находится в пределах секции
    if (offset >= target->size) {
        printf("[ERROR] Релокация #%d: смещение 0x%08x вне границ секции %s (размер: 0x%08x)\n",
               reloc_index, offset, target->name, target->size);
        return;
    }
    
    // Получаем указатель на место для применения релокации
    uint8_t *location = target->alloc_addr + offset;
    uint32_t *location32 = (uint32_t *)location;
    uint16_t *location16 = (uint16_t *)location;
    uint8_t *location8 = (uint8_t *)location;

    

    addend = *((int32_t*)location);




    
    uint8_t *symbol_addr = get_symbol_absolute_addr(ctx, sym_index);
    char *symbol_name = get_symbol_name(ctx, sym_index);
    if(type == R_XTENSA_SLOT0_OP) return;
    printf("[RELOCATION #%d]\n", reloc_index);
    printf("  Тип: %s (%u)\n", get_relocation_type_name(type), type);
    printf("  Секция: %s (адрес в ОЗУ: %p, размер: 0x%08zx)\n", 
           target->name, (void*)target->alloc_addr, target->size);
    printf("  Смещение в секции: 0x%08x\n", offset);
    printf("  Адрес релокации в ОЗУ: %p\n", (void*)location);
    printf("  Символ: %s (индекс: %u, адрес: %p)\n", 
           symbol_name, sym_index, (void*)symbol_addr);
    printf("  Адденд: 0x%08x (%d)\n", addend, addend);
    
    // Для неразрешенных символов (NULL) показываем предупреждение
    if (symbol_addr == NULL && sym_index != 0) {
        printf("  ВНИМАНИЕ: Символ не разрешен (адрес = NULL). Возможно, внешняя ссылка.\n");
        return; // Не применяем релокацию для неразрешенных символов
    }
    
    switch(type) {
        case R_XTENSA_32: {
            // Абсолютная 32-битная релокация: S + A
            uint32_t value = (uint32_t)(uintptr_t)symbol_addr + addend;
            printf("  Формула: S + A = %p + 0x%08x\n", (void*)symbol_addr, addend);
            printf("  Результат: 0x%08x\n", value);
            printf("  Записываем 0x%08x по адресу %p\n", 
                   value, (void*)location32);
            *location32 = value;
            break;
        }
        
        case R_XTENSA_32_PCREL: {
            // PC-относительная 32-битная релокация: S + A - P
            // В Xtensa PC указывает на следующую инструкцию
            uint32_t pc = (uint32_t)(uintptr_t)location + 4; // PC указывает на следующую инструкцию
            uint32_t value = (uint32_t)(uintptr_t)symbol_addr + addend - pc;
            printf("  PC = P + 4 = %p + 4 = 0x%08x\n", (void*)location, pc);
            printf("  Формула: S + A - PC = %p + 0x%08x - 0x%08x\n", 
                   (void*)symbol_addr, addend, pc);
            printf("  Результат: 0x%08x\n", value);
            printf("  Записываем 0x%08x по адресу %p\n", value, (void*)location32);
            *location32 = value;
            break;
        }
        
        case R_XTENSA_SLOT0_OP:
        case R_XTENSA_SLOT1_OP:
        case R_XTENSA_SLOT2_OP:
        case R_XTENSA_SLOT3_OP:
        case R_XTENSA_SLOT4_OP:
        case R_XTENSA_SLOT5_OP:
        case R_XTENSA_SLOT6_OP:
        case R_XTENSA_SLOT7_OP:
        case R_XTENSA_SLOT8_OP:
        case R_XTENSA_SLOT9_OP:
        case R_XTENSA_SLOT10_OP:
        case R_XTENSA_SLOT11_OP:
        case R_XTENSA_SLOT12_OP:
        case R_XTENSA_SLOT13_OP:
        case R_XTENSA_SLOT14_OP: {
            uint32_t slot = type - R_XTENSA_SLOT0_OP;
            // Для слотов инструкций Xtensa: обычно S + A - P, но упрощенно S + A
            uint32_t value = (uint32_t)(uintptr_t)symbol_addr + addend;
            printf("  Слот инструкции: slot%d\n", slot);
            printf("  Формула (упрощенно): S + A = %p + 0x%08x\n", (void*)symbol_addr, addend);
            printf("  Результат: 0x%08x\n", value);
            printf("  Записываем 0x%08x в слот %d по адресу %p\n", 
                   value, slot, (void*)location32);
            // *location32 = value;
            
            apply_xtensa_slot0_reloc(location8, value);

            break;
        }
        
        case R_XTENSA_OP0:
        case R_XTENSA_OP1:
        case R_XTENSA_OP2: {
            uint32_t op_field = type - R_XTENSA_OP0;
            uint32_t current_instr = *location32;
            uint32_t op0, op1, op2;
            
            decode_xtensa_instruction(current_instr, &op0, &op1, &op2);
            
            printf("  Текущая инструкция: 0x%08x (op0=0x%x, op1=0x%x, op2=0x%x)\n",
                   current_instr, op0, op1, op2);
            
            // Для OP релокаций: значение символа помещается в поле опкода
            uint32_t new_op = ((uint32_t)(uintptr_t)symbol_addr + addend) & 0xF;
            
            if (op_field == 0) {
                op0 = new_op;
                printf("  Обновляем op0 на 0x%x (S + A = %p + 0x%08x)\n", 
                       new_op, (void*)symbol_addr, addend);
            } else if (op_field == 1) {
                op1 = new_op;
                printf("  Обновляем op1 на 0x%x (S + A = %p + 0x%08x)\n", 
                       new_op, (void*)symbol_addr, addend);
            } else {
                op2 = new_op;
                printf("  Обновляем op2 на 0x%x (S + A = %p + 0x%08x)\n", 
                       new_op, (void*)symbol_addr, addend);
            }
            
            uint32_t new_instr = (op2 << 8) | (op1 << 4) | op0;
            printf("  Новая инструкция: 0x%08x\n", new_instr);
            printf("  Записываем по адресу %p\n", (void*)location32);
            *location32 = new_instr;
            break;
        }
        
        case R_XTENSA_GLOB_DAT:
        case R_XTENSA_JMP_SLOT: {
            printf("  Динамическая релокация: запись адреса символа\n");
            printf("  Записываем %p по адресу %p\n", (void*)symbol_addr, (void*)location32);
            *location32 = (uint32_t)(uintptr_t)symbol_addr;
            break;
        }
        
        case R_XTENSA_RELATIVE: {
            // Относительная релокация: адрес текущей секции + адденд
            uint32_t value = (uint32_t)(uintptr_t)target->alloc_addr + addend;
            printf("  Относительная релокация: адрес секции + адденд\n");
            printf("  Формула: addr(section) + A = %p + 0x%08x\n", 
                   (void*)target->alloc_addr, addend);
            printf("  Результат: 0x%08x\n", value);
            printf("  Записываем 0x%08x по адресу %p\n", value, (void*)location32);
            *location32 = value;
            break;
        }
        
        case R_XTENSA_NONE:
            printf("  Пустая релокация - ничего не делаем\n");
            break;
            
        default:
            printf("  ВНИМАНИЕ: Необработанный тип релокации! Пропускаем.\n");
            break;
    }
    
    printf("\n");
}

void process_rel(Elf32_Rel *rel, SectionBuffer *target, 
                SectionBuffer *reloc_section, ELFContext *ctx, int reloc_index) {
    Elf32_Rela rela;
    rela.r_offset = rel->r_offset;
    rela.r_info = rel->r_info;
    
    // Для REL релокаций адденд уже содержится в данных по месту релокации
    if (rel->r_offset < target->size) {
        uint32_t *location = (uint32_t *)(target->alloc_addr + rel->r_offset);
        rela.r_addend = *location;  // Адденд уже в данных
    } else {
        rela.r_addend = 0;
    }
    
    printf("[REL RELOCATION #%d] (адденд читается из целевого места)\n", reloc_index);
    process_relocation(&rela, target, reloc_section, ctx, reloc_index);
}

void apply_relocations(ELFContext *ctx) {
    printf("\n");
    print_separator(60);
    printf("ПРИМЕНЕНИЕ РЕЛОКАЦИЙ:\n");
    print_separator(60);
    printf("\n");
    
    int total_relocations = 0;
    int skipped_relocations = 0;
    
    for (int i = 0; i < ctx->section_count; i++) {
        SectionBuffer *section = &ctx->sections[i];
        
        // Ищем только секции релокаций
        if (!is_relocation_section(&section->header)) {
            continue;
        }
        
        // Загружаем секцию релокаций в память (если еще не загружена)
        if (!section->is_loaded) {
            section->data = malloc(section->size);
            if (!section->data) {
                fprintf(stderr, "Failed to allocate memory for relocation section '%s'\n", section->name);
                continue;
            }
            
            fseek(ctx->file, section->header.sh_offset, SEEK_SET);
            if (fread(section->data, 1, section->size, ctx->file) != section->size) {
                fprintf(stderr, "Failed to read relocation section '%s'\n", section->name);
                free(section->data);
                section->data = NULL;
                continue;
            }
            
            section->alloc_addr = section->data;
            section->is_loaded = 1;
        }
        
        printf("Найдена секция релокаций: %s\n", section->name);
        printf("Тип: %s\n", 
               section->header.sh_type == SHT_RELA ? "RELA" : "REL");
        printf("Размер: %u байт\n", section->header.sh_size);
        
        // В поле sh_info содержится индекс целевой секции в исходной таблице заголовков
        uint32_t target_section_index = section->header.sh_info;
        
        if (target_section_index >= ctx->elf_header.e_shnum) {
            printf("  ОШИБКА: Неверный индекс целевой секции: %u\n\n", target_section_index);
            skipped_relocations++;
            continue;
        }
        
        // Получаем заголовок целевой секции из исходной таблицы
        Elf32_Shdr *target_section_hdr = &ctx->section_headers[target_section_index];
        char *target_section_name = &ctx->section_names[target_section_hdr->sh_name];
        
        // Ищем загруженную целевую секцию
        SectionBuffer *target_buffer = NULL;
        for (int j = 0; j < ctx->section_count; j++) {
            if (ctx->sections[j].header.sh_offset == target_section_hdr->sh_offset) {
                target_buffer = &ctx->sections[j];
                break;
            }
        }
        
        if (!target_buffer) {
            printf("  ОШИБКА: Не найден буфер для целевой секции '%s'\n\n", 
                   target_section_name);
            skipped_relocations++;
            continue;
        }
        
        // Проверяем, загружена ли целевая секция в память
        if (!target_buffer->is_loaded) {
            printf("  ПРОПУСК: Целевая секция '%s' не загружена в память\n", target_section_name);
            printf("           Релокации не будут применены к этой секции\n\n");
            skipped_relocations++;
            continue;
        }
        
        printf("Целевая секция: %s (адрес в ОЗУ: %p, размер: %zu)\n", 
               target_section_name, 
               (void*)target_buffer->alloc_addr,
               target_buffer->size);
        printf("Флаги: 0x%08x", target_buffer->header.sh_flags);
        if (target_buffer->header.sh_flags & SHF_EXECINSTR) printf(" EXECINSTR");
        if (target_buffer->header.sh_flags & SHF_WRITE) printf(" WRITE");
        if (target_buffer->header.sh_flags & SHF_ALLOC) printf(" ALLOC");
        printf("\n");
        
        if (section->header.sh_type == SHT_RELA) {
            Elf32_Rela *relocations = (Elf32_Rela *)section->data;
            size_t num_relocations = section->size / sizeof(Elf32_Rela);
            
            printf("Количество релокаций RELA: %zu\n\n", num_relocations);
            
            for (size_t j = 0; j < num_relocations; j++) {
                process_relocation(&relocations[j], target_buffer, section, ctx, total_relocations);
                total_relocations++;
            }
        } else if (section->header.sh_type == SHT_REL) {
            Elf32_Rel *relocations = (Elf32_Rel *)section->data;
            size_t num_relocations = section->size / sizeof(Elf32_Rel);
            
            printf("Количество релокаций REL: %zu\n\n", num_relocations);
            
            for (size_t j = 0; j < num_relocations; j++) {
                process_rel(&relocations[j], target_buffer, section, ctx, total_relocations);
                total_relocations++;
            }
        }
        
        print_separator(40);
        printf("\n");
    }
    
    printf("СТАТИСТИКА РЕЛОКАЦИЙ:\n");
    printf("  Применено релокаций: %d\n", total_relocations);
    printf("  Пропущено релокаций: %d\n", skipped_relocations);
    printf("\n");
}

void print_section_info(ELFContext *ctx) {
    printf("\n");
    print_separator(60);
    printf("СЕКЦИИ ЗАГРУЖЕННЫЕ В ПАМЯТЬ:\n");
    print_separator(60);
    printf("\n");
    
    printf("%-20s %-16s %-12s %-10s %s\n", 
           "Имя секции", "Адрес в ОЗУ", "Размер", "Флаги", "Тип");
    print_separator(80);
    
    int alloc_section_count = 0;
    size_t total_size = 0;
    
    for (int i = 0; i < ctx->section_count; i++) {
        SectionBuffer *section = &ctx->sections[i];
        
        // Показываем только секции, загруженные в память
        if (!section->is_loaded) {
            continue;
        }
        
        alloc_section_count++;
        total_size += section->size;
        
        printf("%-20s %-16p %-12zu ",
               section->name,
               (void*)section->alloc_addr,
               section->size);
        
        // Флаги
        if (section->header.sh_flags & SHF_EXECINSTR) printf("X");
        if (section->header.sh_flags & SHF_WRITE) printf("W");
        if (section->header.sh_flags & SHF_ALLOC) printf("A");
        printf(" ");
        
        // Тип секции
        printf("%s", get_section_type_name(section->header.sh_type));
        
        printf("\n");
    }
    
    printf("\nИТОГО:\n");
    printf("  Секций в памяти: %d\n", alloc_section_count);
    printf("  Всего памяти: %zu байт\n", total_size);
    printf("\n");
}

void cleanup(ELFContext *ctx) {
    if (ctx->section_names) free(ctx->section_names);
    if (ctx->section_headers) free(ctx->section_headers);
    
    for (int i = 0; i < ctx->section_count; i++) {
        if (ctx->sections[i].data) {
            free(ctx->sections[i].data);
        }
    }
    
    for (int i = 0; i < ctx->symbol_count; i++) {
        if (ctx->symbols[i].name) {
            free(ctx->symbols[i].name);
        }
    }
    
    if (ctx->file) fclose(ctx->file);
    memset(ctx, 0, sizeof(ELFContext));
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <elf_file>\n", argv[0]);
        fprintf(stderr, "Пример: %s firmware.elf\n", argv[0]);
        return 1;
    }

    ELFContext ctx = {0};
    
    printf("Прототип загрузчика ELF приложений для Xtensa (ESP32)\n");
    printf("Файл: %s\n\n", argv[1]);
    
    // 1. Парсим ELF файл
    if (parse_elf(argv[1], &ctx) != 0) {
        fprintf(stderr, "Ошибка при разборе ELF файла\n");
        return 1;
    }
    
    // 2. Загружаем секции в память (используя malloc)
    int loaded = load_sections_to_memory(&ctx);
    if (loaded <= 0) {
        fprintf(stderr, "Не удалось загрузить секции в память\n");
        cleanup(&ctx);
        return 1;
    }
    
    // 3. Загружаем таблицу символов
    if (load_symbols(&ctx) != 0) {
        printf("Предупреждение: не удалось загрузить таблицу символов\n");
    }
    
    // 4. Вычисляем абсолютные адреса символов
    if (ctx.symbol_count > 0) {
        calculate_symbol_addresses(&ctx);
    }
    
    printf("ELF файл успешно загружен!\n");
    printf("Тип файла: %u, Архитектура: %u (Xtensa)\n", 
           ctx.elf_header.e_type, ctx.elf_header.e_machine);
    printf("Точка входа: 0x%08x\n", ctx.entry_point);
    printf("Загружено секций в память: %d\n", loaded);
    printf("Загружено символов: %d\n\n", ctx.symbol_count);
    
    // Выводим информацию о секциях в памяти
    print_section_info(&ctx);
    
    // Применяем релокации
    apply_relocations(&ctx);
    
    // Выводим точку входа как указатель на функцию
    if (ctx.entry_point != 0) {
        printf("Точка входа программы: 0x%08x\n", ctx.entry_point);
        printf("Для запуска программы вызовите функцию по адресу: 0x%08x\n", ctx.entry_point);
    }
    
    printf("\nЗагрузка завершена успешно!\n");
    printf("Программа загружена в память и готова к выполнению.\n");
    
    cleanup(&ctx);
    return 0;
}