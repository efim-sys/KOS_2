#include <stdint.h>
#include <stdlib.h>

int var;

int (*quad)(int) = (void*) 0x1688U;

int triple(int a) {
    return a*3;
}

void do_magic();

float main() {
    do_magic();
    return triple(27)+quad(var);
}