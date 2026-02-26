#include "elf_functions.h"
#include <stdlib.h>
#include <string.h>

ELFContext* load_elf_file(const char* filename) {
    ELFContext *ctx = calloc(1, sizeof(ELFContext));

    if (parse_elf(filename, ctx) != 0) {
        err_pf("Ошибка при разборе ELF файла\n");
        return 1;
    }

    int sections_loaded = load_sections_to_memory(ctx);
    if (sections_loaded == 0) {
        err_pf("Не удалось загрузить секции в память\n");
        cleanup(&ctx);
        return 1;
    }

    if (load_symbols(ctx) != 0) {
        err_pf("Предупреждение: не удалось загрузить таблицу символов\n");
    }

    if (ctx->symbol_count > 0) {
        calculate_symbol_addresses(ctx);
    }

    log_pf("ELF файл успешно загружен!\n");
    log_pf("Архитектура: %u (Xtensa)\n", ctx->elf_header.e_machine);
    log_pf("Загружено секций в память: %d\n", sections_loaded);
    log_pf("Загружено символов: %d\n\n", ctx->symbol_count);

    apply_relocations(ctx);

    return ctx;
}

int find_section_by_name(ELFContext *ctx, const char *name) {
    for (int i = 0; i < ctx->section_count; i++) {
        if (strcmp(ctx->sections[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void* get_text_section(ELFContext *ctx) {
    int text_section_id = find_section_by_name(ctx, ".text");
    return ctx->sections[text_section_id].data;
}

int is_metadata_section(Elf32_Shdr *shdr) {
    return  shdr->sh_type == SHT_SYMTAB ||
            shdr->sh_type == SHT_STRTAB ||
            shdr->sh_type == SHT_RELA ||
            shdr->sh_type == SHT_REL;
}

int read_section_header(ELFContext *ctx) {
    // Переход к таблице заголовков секций
    fseek(ctx->file, ctx->elf_header.e_shoff, SEEK_SET);
    
    ctx->section_headers = malloc(ctx->elf_header.e_shnum * sizeof(Elf32_Shdr));
    if (!ctx->section_headers) return -err_pf("Memory allocation failed\n");

    if (fread(ctx->section_headers, sizeof(Elf32_Shdr), 
              ctx->elf_header.e_shnum, ctx->file) != ctx->elf_header.e_shnum) {
        free(ctx->section_headers);
        return -err_pf("Failed to read section headers\n");;
    }

    return 0;
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

    free(ctx);
}

int read_section_names(ELFContext *ctx) {
    Elf32_Shdr *strtab_header = &ctx->section_headers[ctx->elf_header.e_shstrndx];
    ctx->section_names = malloc(strtab_header->sh_size);
    if (!ctx->section_names) {
        free(ctx->section_headers);
        return -err_pf("Memory allocation failed\n");
    }

    fseek(ctx->file, strtab_header->sh_offset, SEEK_SET);
    if (fread(ctx->section_names, 1, strtab_header->sh_size, ctx->file) != strtab_header->sh_size) {
        free(ctx->section_names);
        free(ctx->section_headers);
        return -err_pf("Failed to read section names\n");
    }

    return 0;
}

void set_section_parameters(ELFContext *ctx) {
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
}


int load_sections_to_memory(ELFContext *ctx) {
    log_pf("Загрузка секций в память:\n");
    
    int loaded_count = 0;
    
    for (int i = 0; i < ctx->section_count; i++) {
        SectionBuffer *section = &ctx->sections[i];
        
        if (is_metadata_section(&section->header)) {
            log_pf("[SKIP] Пропускаем метаданные: '%s'\n", 
                   section->name);
            continue;
        }
        
        // Не загружать в память если, нет флага ALLOC
        if (!(section->header.sh_flags & SHF_ALLOC)) {
            log_pf("[SKIP] Пропускаем секцию без флага ALLOC: '%s'\n", section->name);
            continue;
        }
        
        section->data = malloc(section->size);
        if (!section->data) {
            err_pf("Failed to allocate %zu bytes for section '%s'\n", 
                    section->size, section->name);
            continue;
        }
        
        // Для секций .bss заполняем нулями
        if (section->header.sh_type == SHT_NOBITS) {
            memset(section->data, 0, section->size);
        } else {
            fseek(ctx->file, section->header.sh_offset, SEEK_SET);
            if (fread(section->data, 1, section->size, ctx->file) != section->size) {
                err_pf("Failed to read data for section '%s'\n", section->name);
                free(section->data);
                section->data = NULL;
                continue;
            }
        }
        
        section->alloc_addr = section->data;
        section->is_loaded = 1;
        
        log_pf("[LOADED] Секция: %s\n", section->name);
        log_pf("         Адрес в ОЗУ: %p\n", (void*)section->alloc_addr);
        log_pf("         Размер: %zu байт\n", section->size);
        log_pf("         Флаги: 0x%08x", section->header.sh_flags);
        if (section->header.sh_flags & SHF_EXECINSTR) log_pf(" EXECINSTR");
        if (section->header.sh_flags & SHF_WRITE) log_pf(" WRITE");
        if (section->header.sh_flags & SHF_ALLOC) log_pf(" ALLOC");
        log_pf("\n\n");
        
        loaded_count++;
    }
    
    return loaded_count;
}

int parse_elf(const char *filename, ELFContext *ctx) {
    ctx->file = fopen(filename, "rb");
    if (!ctx->file) return -err_pf("Failed to open file\n");

    if (fread(&ctx->elf_header, sizeof(Elf32_Ehdr), 1, ctx->file) != 1) {
        fclose(ctx->file);
        return -err_pf("Failed to read ELF header\n");
    }

    // Магические числа
    if (memcmp(ctx->elf_header.e_ident, ELFMAG, SELFMAG) != 0) {
        fclose(ctx->file);
        return -err_pf("Not a valid ELF file\n");
    }

    // Проверка архитектуры
    if (ctx->elf_header.e_machine != 0x5E) {
        fclose(ctx->file);
        return -err_pf("Not an Xtensa/ESP32 ELF file (e_machine = 0x%x)\n", ctx->elf_header.e_machine);;
    }

    // Точка входа не используется, т.к. файл не полностью слинкован
    // ctx->entry_point = ctx->elf_header.e_entry;

    if(read_section_header(ctx) != 0) {
        fclose(ctx->file);
        return -err_pf("Section headers reading error\n");
    }

    if(read_section_names(ctx) != 0) {
        fclose(ctx->file);
        return -err_pf("Section names reading error\n");
    }

    set_section_parameters(ctx);

    return 0;
}

int load_symbols(ELFContext *ctx) {
    ctx->symbol_count = 0;
    
    // Поиск таблицы символов (.symtab)
    for (int i = 0; i < ctx->elf_header.e_shnum; i++) {
        Elf32_Shdr *shdr = &ctx->section_headers[i];
        
        if (shdr->sh_type == SHT_SYMTAB) {
            char *section_name = &ctx->section_names[shdr->sh_name];
            log_pf("[LOADER] Найдена таблица символов: %s (размер: %u)\n", 
                   section_name, shdr->sh_size);
            
            // Временный буфер для таблицы символов
            uint8_t *symtab_data = malloc(shdr->sh_size);
            if (!symtab_data) {
                err_pf("Failed to allocate memory for symbol table\n");
                return -1;
            }
            
            // Чтение таблицы символов
            fseek(ctx->file, shdr->sh_offset, SEEK_SET);
            if (fread(symtab_data, 1, shdr->sh_size, ctx->file) != shdr->sh_size) {
                free(symtab_data);
                return -err_pf("Failed to read symbol table\n");
            }
            
            Elf32_Sym *symtab = (Elf32_Sym *)symtab_data;
            uint32_t num_symbols = shdr->sh_size / sizeof(Elf32_Sym);
            
            // Получение таблицы строк имен символов
            Elf32_Shdr *string_table_hdr = &ctx->section_headers[shdr->sh_link];
            char *string_table = malloc(string_table_hdr->sh_size);

            if(!string_table) return -err_pf("Cannot allocate memory for string table");

            fseek(ctx->file, string_table_hdr->sh_offset, SEEK_SET);
            fread(string_table, 1, string_table_hdr->sh_size, ctx->file);
            
            log_pf("[LOADER] Загружаем символы (%u записей):\n", num_symbols);
            
            for (uint32_t j = 0; j < num_symbols && ctx->symbol_count < MAX_SYMBOLS; j++) {
                SymbolInfo *sym = &ctx->symbols[ctx->symbol_count];
                sym->value = symtab[j].st_value;
                sym->size = symtab[j].st_size;
                sym->type = ELF32_ST_TYPE(symtab[j].st_info);
                sym->bind = ELF32_ST_BIND(symtab[j].st_info);
                sym->shndx = symtab[j].st_shndx;
                sym->absolute_addr = NULL;  // Адрес пока не вычислен
                
                if (symtab[j].st_name > 0 && symtab[j].st_name < string_table_hdr->sh_size) {
                    sym->name = strdup(&string_table[symtab[j].st_name]);
                } else {
                    sym->name = strdup("unnamed");
                }
                
                ctx->symbol_count++;
            }
            
            free(string_table);
            free(symtab_data);
            
            log_pf("[LOADER] Загружено символов: %d\n", ctx->symbol_count);
            return 0;
        }
    }
    
    log_pf("[LOADER] Таблица символов не найдена\n");
    return -1;
}

int set_general_symbol_adress(ELFContext *ctx, SymbolInfo *sym) {
    if (sym->shndx >= ctx->elf_header.e_shnum) {
        return -err_pf("  %-30s: ERROR (некорректный индекс секции: %u)\n", sym->name, sym->shndx);
    }

    Elf32_Shdr *section_hdr = &ctx->section_headers[sym->shndx];
    char *section_name = &ctx->section_names[section_hdr->sh_name];
    
    // Поиск секции в массиве загруженных секций
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
        log_pf("  %-30s: %p (секция: %s, смещение: 0x%08x)\n", 
            sym->name, (void*)sym->absolute_addr, section_name, sym->value);
    } else {
        // Секция не загружена в память
        sym->absolute_addr = NULL;
        log_pf("  %-30s: NOT LOADED (секция %s не загружена)\n", 
            sym->name, section_name);
    }
    return 0;
}


int calculate_symbol_addresses(ELFContext *ctx) {
    log_pf("[LOADER] Вычисление абсолютных адресов символов:\n");
    
    for (int i = 0; i < ctx->symbol_count; i++) {
        SymbolInfo *sym = &ctx->symbols[i];

        switch(sym->shndx) {
            case SHN_UNDEF:
                sym->absolute_addr = NULL;
                printf("  %-30s: EXTERNAL (секция: UNDEF)\n", sym->name);
                break;
            case SHN_ABS:
                sym->absolute_addr = (uint8_t*)(uintptr_t)sym->value;
                printf("  %-30s: ABSOLUTE (адрес: %p)\n", sym->name, (void*)sym->absolute_addr);
                break;
            default:
                set_general_symbol_adress(ctx, sym);
                break;
        }
    }
    return 0;
}

int is_relocation_section(Elf32_Shdr *shdr) {
    return (shdr->sh_type == SHT_RELA || shdr->sh_type == SHT_REL);
}


void apply_relocations(ELFContext *ctx) {   
    int total_relocations = 0;
    int skipped_relocations = 0;
    
    for (int i = 0; i < ctx->section_count; i++) {
        SectionBuffer *section = &ctx->sections[i];
        
        if (!is_relocation_section(&section->header)) continue;
        
        // Загружаем секцию релокаций в память (если еще не загружена)
        if (!section->is_loaded) {
            section->data = malloc(section->size);
            if (!section->data) {
                err_pf("Failed to allocate memory for relocation section '%s'\n", section->name);
                continue;
            }
            
            fseek(ctx->file, section->header.sh_offset, SEEK_SET);
            if (fread(section->data, 1, section->size, ctx->file) != section->size) {
                err_pf("Failed to read relocation section '%s'\n", section->name);
                free(section->data);
                section->data = NULL;
                continue;
            }
            
            section->alloc_addr = section->data;
            section->is_loaded = 1;
        }
        
        log_pf("Найдена секция релокаций: %s\n", section->name);
        log_pf("Тип: %s\n", section->header.sh_type == SHT_RELA ? "RELA" : "REL");
        log_pf("Размер: %u байт\n", section->header.sh_size);
        
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
                if(process_relocation(&relocations[j], target_buffer, section, ctx, total_relocations) == 0) {
                    total_relocations++;
                }
                else {
                    skipped_relocations++;
                }
                
            }
        }

    }
    
    printf("СТАТИСТИКА РЕЛОКАЦИЙ:\n");
    printf("  Применено релокаций: %d\n", total_relocations);
    printf("  Пропущено релокаций: %d\n", skipped_relocations);
    printf("\n");
}

uint8_t* get_symbol_absolute_addr(ELFContext *ctx, uint32_t sym_index) {
    return ctx->symbols[sym_index].absolute_addr;
}

char* get_symbol_name(ELFContext *ctx, uint32_t sym_index) {
    return ctx->symbols[sym_index].name;
}


int process_relocation(Elf32_Rela *rela, SectionBuffer *target, 
                       SectionBuffer *reloc_section, ELFContext *ctx, int reloc_index) {
    uint32_t type = ELF32_R_TYPE(rela->r_info);

    if(type != R_XTENSA_32) return -1; // Обрабатывается только тип релокаций R_XTENSA_32
    
    uint32_t offset = rela->r_offset;
    uint32_t sym_index = ELF32_R_SYM(rela->r_info);
    int32_t addend = rela->r_addend;

    // Проверяем, что смещение находится в пределах секции
    if (offset >= target->size) {
        printf("[ERROR] Релокация #%d: смещение 0x%08x вне границ секции %s (размер: 0x%08x)\n",
               reloc_index, offset, target->name, target->size);
        return -1;
    }
    
    // Получаем указатель на место для применения релокации
    uint32_t *location = target->alloc_addr + offset;
        

    addend = *((int32_t*)location);

    
    uint8_t *symbol_addr = get_symbol_absolute_addr(ctx, sym_index);
    char *symbol_name = get_symbol_name(ctx, sym_index);

    
    printf("[RELOCATION #%d]\n", reloc_index);
    printf("  Тип: (%u)\n", type);
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
        return -1; // Не применяем релокацию для неразрешенных символов
    }
    
    // Абсолютная 32-битная релокация: S + A
    uint32_t value = (uint32_t)symbol_addr + addend;
    printf("  Формула: S + A = %p + 0x%08x\n", (void*)symbol_addr, addend);
    printf("  Результат: 0x%08x\n", value);
    printf("  Записываем 0x%08x по адресу %p\n", 
            value, (void*)location);
    *location = value;    
    
    printf("\n");

    return 0;
}