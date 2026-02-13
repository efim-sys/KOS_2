#include "../src/launcher/export.h"

int main(struct exp_os*);
int __attribute__((section(".start"))) _start(struct exp_os* os) {
    return main(os);
}



int main(struct exp_os* os) {
    int w = 240;
    int h = 280;

    for(unsigned int i = 0; i < w*h; i++) {
        os->fb[i] = i;
    }

    const int array[] = {0xDEADBEEF, 0xDEADBEEF, 0xDEADBEEF, 0xDEADBEEF};

    for(unsigned int i = 0; i < 4; i++) {
        os->putd(array[i]);
    }

    os->display();

    return h;
}