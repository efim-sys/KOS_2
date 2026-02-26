#include <stdio.h>

#include "elf_functions.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        log_pf("Использование: %s <elf_file>\n", argv[0]);
        log_pf("Пример: %s firmware.elf\n", argv[0]);
        return 1;
    }

    ELFContext *ctx = load_elf_file(argv[1]);

    int(*fn_main)(void* param) = get_text_section(ctx);

    printf("fm_nain [%p]\n\n", fn_main);

    cleanup(ctx);

    return 0;
}