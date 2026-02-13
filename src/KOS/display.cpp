#include "display.h"
#include "defines.h"

LGFX::LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();

      cfg.spi_host = SPI2_HOST;
      cfg.freq_write = 80000000;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = 13;
      cfg.pin_mosi = 11;
      cfg.pin_dc   = 12;

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();

      cfg.pin_cs           =    14;
      cfg.pin_rst          =    21;
      
      cfg.panel_width      =   240;

      #ifdef IPS169
      cfg.panel_height     =   300;
      #else
      cfg.panel_height     =   240;
      #endif

      cfg.invert           = true;

      _panel_instance.config(cfg);
    }
    {
      auto cfg = _light_instance.config();

      cfg.pin_bl = 3;
      cfg.invert = false;
      cfg.freq   = 44100;
      cfg.pwm_channel = 7;

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    setPanel(&_panel_instance);
  }



LGFX display;

LGFX_Sprite canvas(&display);
namespace KOS {
  void initDisplay() {
      display.init();
      display.setRotation(2);
      display.setBrightness(255);
      display.clear(TFT_BLACK);
      
      canvas.setPsram(true);
      #ifndef IPS169
      canvas.createSprite(display.width(), display.height());
      #else
      canvas.createSprite(display.width(), display.height()-20);
      #endif
  }
}