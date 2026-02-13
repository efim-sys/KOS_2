#pragma once

#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ST7789      _panel_instance;
  lgfx::Bus_SPI        _bus_instance;
  lgfx::Light_PWM     _light_instance;

public:
  LGFX(void);
};

extern LGFX display;

extern LGFX_Sprite canvas;