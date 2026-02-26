#pragma once

#include "structures.h"

void cleanup(ELFContext *ctx);
int read_section_header(ELFContext *ctx);
int read_section_names(ELFContext *ctx);
void set_section_parameters(ELFContext *ctx);
int parse_elf(const char *filename, ELFContext *ctx);
int load_sections_to_memory(ELFContext *ctx);
int load_symbols(ELFContext *ctx);
int calculate_symbol_addresses(ELFContext *ctx);
void apply_relocations(ELFContext *ctx);
ELFContext* load_elf_file(const char* filename);
int find_section_by_name(ELFContext *ctx, const char *name);
void* get_text_section(ELFContext *ctx);