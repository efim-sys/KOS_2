SECTIONS
{
    /* 1. Код */
    .text : {
        . = ALIGN(4);
        *(.text .text.* .gnu.linkonce.t.*)
        . = ALIGN(4);
    }

    /* 2. Инициализированные данные */
    .data : {
        . = ALIGN(4);
        *(.data .data.* .gnu.linkonce.d.*)
        . = ALIGN(4);
    }

    /* 3. Неинициализированные данные */
    .bss : {
        . = ALIGN(4);
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);
    }

    /* 4. Константы (Read-only data) */
    .rodata : {
        . = ALIGN(4);
        *(.rodata .rodata.* .gnu.linkonce.r.*)
        . = ALIGN(4);
    }

    /* 5. Удаление ненужных секций */
    /DISCARD/ :
    {
        *(.xtensa.info)
        *(.xt.prop)
        *(.xt.prop.*)
        *(.xt.lit)
        *(.xt.lit.*)
        *(.note.GNU-stack)
        *(.comment)
        /* Если вам не нужна отладочная информация, можно раскомментировать: */
        *(.debug*)
    }
}