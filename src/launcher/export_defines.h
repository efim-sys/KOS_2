enum {
  BTN_UP = 0,
  BTN_DOWN,
  BTN_BOOT,
  JOY_UP,
  JOY_DOWN,
  JOY_LEFT,
  JOY_RIGHT,
  JOY_CENTER,

  LED_FLASH = 9,
  SPEAKER = 10
};

// enum {
//   PIN_BTN_UP = 8,
//   PIN_BTN_DOWN = 9,
//   PIN_BTN_BOOT = 0,
//   PIN_JOY_UP = 5,
//   PIN_JOY_DOWN = 7,
//   PIN_JOY_LEFT = 15,
//   PIN_JOY_RIGHT = 6,
//   PIN_JOY_CENTER = 16
// };

#define LOW  0x0
#define HIGH 0x1

#define INPUT 0x01
#define OUTPUT            0x03
#define PULLUP            0x04
#define INPUT_PULLUP      0x05
#define PULLDOWN          0x08
#define INPUT_PULLDOWN    0x09
#define OPEN_DRAIN        0x10
#define OUTPUT_OPEN_DRAIN 0x13
#define ANALOG            0xC0