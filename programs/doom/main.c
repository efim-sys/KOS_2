// #include <math.h>
#include "export.h"
#include "export_defines.h"
// #include "fb_gfx.h"

// struct exp_os *os;

int global_var = 51;

int main(struct exp_os* os) {
    // Canvas canvas = {
    //     .data = os->fb,
    //     .height = 280,
    //     .width = 240
    // };

    // draw_circle(&canvas, 128, 128, 99, COLOR_CYAN);

    // os->display();

    global_var ++;

    return 52 + global_var;
}