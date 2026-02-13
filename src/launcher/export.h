#include <stdint.h>

struct exp_os {
    void (*putc)(char);
    void (*putd)(int);

    uint16_t* fb;

    void (*display)(void);
};