#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "SPIFFS.h"
#include "FS.h"
#include <bits/stdc++.h>
#include "SD_MMC.h"
#include "WiFi.h"
#include "Wire.h"
#include "I2C_eeprom.h"
#include <DS3231.h>
#include "USB.h"
#include "USBCDC.h"
#include "driver/ledc.h"
#include "HTTPUpdate.h"


USBCDC USBSerial;

#define debounceMs 20



#define BTN_UP 0
#define BTN_DOWN 1
#define BTN_BOOT 2
#define JOY_UP 3
#define JOY_DOWN 4
#define JOY_LEFT 5
#define JOY_RIGHT 6
#define JOY_CENTER 7


#undef LED_BUILTIN
#define LED_BUILTIN 45

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ST7789      _panel_instance;
  lgfx::Bus_SPI        _bus_instance;
  lgfx::Light_PWM     _light_instance;

public:

  LGFX(void)
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
};



struct Button{
    uint8_t pin;
    uint32_t lastInterrupt = 0;

    bool inverted = false;

    bool lastValue;
    char name[16] = "Unnamed Button";

    void (*onKeyPress)(uint8_t keyID) = nullptr;
    void (*onKeyRelease)(uint8_t keyID) = nullptr;

    void init(uint8_t _pin, char *_name, void (*intHandler)(), bool _inverted = false) {
        pin = _pin;
        inverted = _inverted;
        pinMode(pin, inverted ? INPUT_PULLUP : INPUT_PULLDOWN);
        lastValue = digitalRead(pin);
        strcpy(name, _name);
        attachInterrupt(pin, intHandler, CHANGE);
        
    }
};

Button buttons[8];
LGFX display;

static LGFX_Sprite canvas(&display);



namespace KOS{   
    I2C_eeprom eeprom(0x50, I2C_DEVICESIZE_24LC32, &Wire);
    DS3231 extRTC;
    bool extRTC_IN = false;

    std::vector<uint8_t> I2C_dev;

    std::vector<uint8_t> scanI2C(TwoWire &wire = Wire) {
      std::vector<uint8_t> devices;
      uint8_t error, address;

      for (address = 1; address < 127; address++) {
          wire.beginTransmission(address);
          error = wire.endTransmission();

          if (error == 0) {  // Device acknowledged
              devices.push_back(address);
          }
      }

      return devices;
    }
    

    namespace autoSleep {
      bool enable = true;
      int32_t timeout = 60000*2;
      // uint32_t timeout = 10000;
      uint64_t lastButtonPress = 0;

      void sleepNow() {
        #ifndef KOROBOCHKA3
        touchSleepWakeUpEnable(T1, 5000);
        #else
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);
        #endif
        
        USBSerial.println("Going to light sleep now");
        byte dBrightness = display.getBrightness();
        display.setBrightness(0);
        esp_light_sleep_start();
        display.setBrightness(dBrightness);
        lastButtonPress = millis();
      }

      void routine(void* arg) {
        while(true) {
          vTaskDelay(3000);
          if(enable and millis() > lastButtonPress+timeout) {
            sleepNow();
          }
          
        }
      }

      void init() {
        // File d = SPIFFS.open("/autoSleep.conf");
        // 0xffffffffUL
        // d.close();
        xTaskCreatePinnedToCore(routine, "AutoSleep", 2048, NULL, 5, NULL, 0);
      }
    }

    #include "interruptDec.h"

    void initI2C() {
      Wire.begin(2, 17);

      eeprom.begin();

      I2C_dev = scanI2C();
      for(uint8_t i : I2C_dev) {
        if(i == 0x68) extRTC_IN = true;
      }
    }

    struct Track{
      fs::FS *soundFS;
      String soundAddress;
    } soundTrack;

    

    struct soundNote {
        int freq;
        float duration;
    };

    TaskHandle_t soundPlayTask;

    void playSound(void*p) {
        pinMode(1, OUTPUT);

        std::vector<soundNote> commands;

        File f = soundTrack.soundFS->open(soundTrack.soundAddress);

        // USBSerial.println(f.size());

        while(f.available()){
            String s = f.readStringUntil('\n');
            int freq; 
            float duration;
            
            std::vector<String> values = {""};        // Create vector of strings

            // We will hold every parameter splitted by space. example: {"v", "0.432", "2.334", "-1.411"}

            for(int i = 0; i < s.length(); i++) {
                if(s[i] == ',') {
                    values.back().trim();
                    values.push_back("");
                }
                else values.back()+=s[i];
            }

            if(s[0] == 'T') {
                freq = values[1].toInt();
                duration = values[2].toFloat();
                commands.push_back(soundNote{freq, duration});
            }
            else if(s[0] == 'D') {
                duration = values[1].toFloat();
                commands.push_back(soundNote{0, duration});
            }
        }

        f.close();

        ledcAttachChannel(1, 340, 10, 3);

        for(soundNote i : commands) {
            ledcWriteTone(1, i.freq);
            vTaskDelay(ceilf(i.duration));
        }

        ledcWriteTone(1, 0);

        pinMode(1, INPUT);

        soundPlayTask = NULL;

        vTaskDelete(NULL);
    }

    void playSound(fs::FS *fs, String filename) {
      #ifndef KOROBOCHKA3
      return;
      #endif
      if(soundPlayTask != NULL) vTaskDelete(soundPlayTask);
      soundTrack.soundFS = fs;
      soundTrack.soundAddress = filename;
      // USBSerial.println(filename);
      xTaskCreatePinnedToCore(playSound, "playSound", 16384, NULL, 4, &soundPlayTask, 1);
    }

    void initButtons() {
        buttons[BTN_UP].    init(8,  (char *) "Button UP",       intHandler0);
        buttons[BTN_DOWN].  init(9,  (char *) "Button DOWN",     intHandler1);
        buttons[BTN_BOOT].  init(0,  (char *) "Button BOOT",     intHandler2, true);
        buttons[JOY_UP].    init(5,  (char *) "JoyStick UP",     intHandler3);
        buttons[JOY_DOWN].  init(7,  (char *) "JoyStick DOWN",   intHandler4);
        buttons[JOY_LEFT].  init(15, (char *) "JoyStick LEFT",   intHandler5);
        buttons[JOY_RIGHT]. init(6,  (char *) "JoyStick RIGHT",  intHandler6);
        buttons[JOY_CENTER].init(16, (char *) "JoyStick CENTER", intHandler7);
    }

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


    void initSPIFFS() {
      SPIFFS.begin();
    }
    #ifndef KOROBOCHKA3
    float getBatteryVoltage() {
      int b = analogRead(4);
      return 1.1*3.1*2.0*(b/4095.0);
    }
    #else

    bool firstMeasuring = true;

    float getBatteryVoltage() {
      pinMode(46, OUTPUT);
      digitalWrite(46, HIGH);
      if(firstMeasuring) {
        delay(10);
        firstMeasuring = false;
      }
      digitalWrite(46, 0);
      delayMicroseconds(25);
      int b = analogRead(4);
      digitalWrite(46, HIGH);
      return 1.1*3.1*2.0*(b/4095.0);
    }
    #endif

    float getBattery() {
      const float highV = 4.2;
      const float lowV  = 3.0;

      return (getBatteryVoltage()-lowV) / (highV-lowV);
    }

    void initRTC() {
      configTime(3600*3, 0, "ntp.ix.ru");
    }

    String getTimeString() {
      struct tm timeinfo;
      getLocalTime(&timeinfo);
      String e = String(asctime(&timeinfo));
      e.trim();
      return e;
    }

    void initWiFi(String ssid, String password) {
      WiFi.mode(WIFI_MODE_AP);
      WiFi.begin(ssid, password);
    }

    void initWiFi(fs::FS &fs = SPIFFS,  String filename = "/wifi.conf") {
      initSPIFFS();

      File config = fs.open(filename);

      if(!config) {
        USBSerial.printf("Error on file open: %s\n", filename.c_str());
        return;
      }

      String ssid = config.readStringUntil('\n');
      String pass = config.readStringUntil('\n');
      ssid.trim();
      pass.trim();
      USBSerial.print(ssid);
      USBSerial.print(pass);

      initWiFi(ssid, pass);

      config.close();
    }

    bool WiFiConnected() {
      return WiFi.status() == WL_CONNECTED;
    }

    void saveWiFiCredentials(String ssid, String pass, fs::FS &fs = SPIFFS,  String filename = "/wifi.conf") {
      initSPIFFS();

      File config = fs.open(filename, "w");

      if(!config) {
        USBSerial.printf("Error on file open: %s\n", filename.c_str());
        return;
      }

      config.println(ssid);
      config.println(pass);

      config.close();
    }

    void getWiFiCredentials(String* ssid, String* pass, fs::FS &fs = SPIFFS,  String filename = "/wifi.conf") {
      initSPIFFS();

      File config = fs.open(filename);

      if(!config) {
        USBSerial.printf("Error on file open: %s\n", filename.c_str());
        return;
      }
      if(ssid != NULL) *ssid = config.readStringUntil('\n');
      if(pass != NULL) *pass = config.readStringUntil('\n');

      config.close();
    }

    bool sdInstalled = false;

    bool initSD(bool skipInsertCheck = false) {
      pinMode(47, INPUT_PULLUP);
      if(!skipInsertCheck) {
        if (digitalRead(47)) {
          USBSerial.println("NOCARD!!!");
        }
      }
      int clk = 39;
      int cmd = 40;
      int d0 = 38;
      int d1 = 48;
      int d2 = 42;
      int d3 = 41;

      SD_MMC.setPins(clk, cmd, d0, d1, d2, d3);

      if (!SD_MMC.begin()) {
        USBSerial.println("Card Mount Failed");
        sdInstalled = false;
        return false;
      } 

      uint8_t cardType = SD_MMC.cardType();

      USBSerial.print("SD_MMC Card Type: ");
      if (cardType == CARD_MMC) {
        USBSerial.println("MMC");
      } else if (cardType == CARD_SD) {
        USBSerial.println("SDSC");
      } else if (cardType == CARD_SDHC) {
        USBSerial.println("SDHC");
      } else {
        USBSerial.println("UNKNOWN");
      }
      sdInstalled  = true;
      return true;
    }

    // Для привязки функции к событию, укажите функцию, для отвязки не указывайте

    void onKeyPress(uint8_t key, void (*function)(uint8_t keyID) = nullptr) {
        buttons[key].onKeyPress = function;
    }

    void onKeyRelease(uint8_t key, void (*function)(uint8_t keyID) = nullptr) {
        buttons[key].onKeyRelease = function;
    }

    void onKeyAll(uint8_t key, void (*function)(uint8_t keyID) = nullptr) {
        buttons[key].onKeyPress = function;
        buttons[key].onKeyRelease = function;
    }

    void detachAllKeys() {
        for(int i = 0; i < 8; i++) {
            KOS::onKeyAll(i);
        }
    }

    bool copyFile(fs::FS &sourceFS, fs::FS &destFS, const char* sourcePath, const char* destPath) {
      // Open source file for reading
      File sourceFile = sourceFS.open(sourcePath, FILE_READ);
      if (!sourceFile) {
        // USBSerial.println("Failed to open source file");
        USBSerial.printf("Failed to open source file: %s", sourcePath);
        return false;
      }

      // Open destination file for writing
      File destFile = destFS.open(destPath, FILE_WRITE);
      if (!destFile) {
        USBSerial.println("Failed to open destination file");
        sourceFile.close();
        return false;
      }

      // Buffer for reading/writing (adjust size as needed)
      uint8_t buffer[512];
      size_t bytesRead;

      // Copy file chunk by chunk
      while ((bytesRead = sourceFile.read(buffer, sizeof(buffer))) > 0) {
        if (destFile.write(buffer, bytesRead) != bytesRead) {
          USBSerial.println("Write error");
          sourceFile.close();
          destFile.close();
          return false;
        }
      }

      // Close files
      sourceFile.close();
      destFile.close();

      return true;
    }

    uint16_t* rotateImageRight(uint16_t* from, int w, int h) {
      uint16_t* addr = (uint16_t*) ps_malloc(w*h*2);
      for(uint32_t i = 0; i < w*h; i++) {
        addr[i] = from[w*h - w - w*(i%h) + i/w];
      }
      return addr;
    }


    void flipImageV(uint16_t* img, int w, int h) {
      for(uint32_t x = 0; x < w; x++) {
        for(uint32_t y = 0; y < h>>1; y++) {
          std::swap(img[x+y*w], img[x+(h-y-1)*w]);
        }
      }
    }

    void flipImageV(uint16_t* img, uint16_t** destination, int w, int h) {
      *destination = (uint16_t*) ps_malloc(w*h*2);
      memcpy(destination, img, w*h*2);
      flipImageV(*destination, w, h);
    }

    void flipImageH(uint16_t* img, int w, int h) {
      for(uint32_t y = 0; y < h; y++) {
        for(uint32_t x = 0; x < w>>1; x++) {
          std::swap(img[x+y*w], img[(w-1-x)+y*w]);
        }
      }
    }

    void flipImageH(uint16_t* img, uint16_t** destination, int w, int h) {
      USBSerial.printf("Allocating %d bytes\n", w*h*2);
      *destination = (uint16_t*) ps_malloc(w*h*2);
      memcpy(*destination, img, w*h*2);
      flipImageH(*destination, w, h);
    }

    char header[0x80];

    uint16_t* readImageBmp(fs::FS &fs, const char* filename, uint32_t* width = NULL, uint32_t* height = NULL) {
      uint16_t* addr;
      
      File image = fs.open(filename);
      image.readBytes(header, 0xB);
      USBSerial.printf("Data offset = %d\n", header[0xA]);
      image.readBytes(header+0xB, header[0xA]-0xA);
      USBSerial.printf("Data offset = %d\n", header[0xA]);

      uint8_t bpp = header[0x1C];

      USBSerial.printf("BPP=%d\n", bpp);

      std::swap(header[15], header[18]);
      std::swap(header[16], header[17]);
      std::swap(header[19], header[22]);
      std::swap(header[20], header[21]);

      uint32_t w =  *(uint32_t*) (header+15);
      uint32_t h =  *(uint32_t*) (header+15+4);

      USBSerial.printf("Filename=%s; w=%d, h=%d\n", filename, w, h);     

      addr = (uint16_t*) ps_malloc(w*h*(bpp/8));

      USBSerial.printf("Allocated %d pixels\n", w*h);

      image.readBytes((char*) addr, w*h*(bpp/8));

      USBSerial.print("Read OK!\n");

      image.close();

      if(bpp == 24) {
        USBSerial.println("BPP=24 -> correcting bpp...");
        uint32_t totalPixels = w*h;
        lgfx::rgb888_t* a24 = (lgfx::rgb888_t*) addr;
        lgfx::rgb565_t* a16 = (lgfx::rgb565_t*) addr;

        for(uint32_t i = 0; i < totalPixels; i++) {
          a16[i] = a24[i];
          
        }
      }

      flipImageV(addr, w, h);

      if(width!=NULL) *width = w;
      if(height!=NULL) *height = h;

      return addr;
    }

    File image;

    void readImageBmp(fs::FS &fs, const char* filename, uint32_t* width, uint32_t* height, uint16_t* addr) {
      // uint64_t tmr = micros();
      image = fs.open(filename);
      
      // USBSerial.printf("File opening time=%d\n", micros()-tmr);
      image.readBytes(header, 0x45);

      std::swap(header[15], header[18]);
      std::swap(header[16], header[17]);
      std::swap(header[19], header[22]);
      std::swap(header[20], header[21]);

      uint32_t w =  *(uint32_t*) (header+15);
      uint32_t h =  *(uint32_t*) (header+15+4);

      uint8_t bpp = header[0x1C];

      // USBSerial.printf("BPP=%d\n", bpp);

      // USBSerial.printf("Filename=%s; w=%d, h=%d\n", filename, w, h);     

      // addr = (uint16_t*) ps_malloc(w*h*2);

      // USBSerial.printf("Allocated %d of uint16\n", w*h);

      image.readBytes((char*) addr, w*h*(bpp/8));

      if(bpp == 24) {
        // USBSerial.println("BPP=24 -> correcting bpp...");
        uint32_t totalPixels = w*h;
        lgfx::rgb888_t* a24 = (lgfx::rgb888_t*) addr;
        lgfx::rgb565_t* a16 = (lgfx::rgb565_t*) addr;

        for(uint32_t i = 0; i < totalPixels; i++) {
          a16[i] = a24[i];
          addr[i] = ntohs(addr[i]);
        }
      }

      // USBSerial.print("Read OK!\n");

      image.close();

      flipImageV(addr, w, h);

      *width = w;
      *height = h;
    }

    namespace KKB {
      int offX = 15;
      int offY = 128;

      LGFX_Sprite textBox(&canvas);

      char buffer[256] = "123";

      int blockSize = 20;

      bool shift = false;

      int activeKey = 0;

      bool endOfInput = false;

      SemaphoreHandle_t keyChange = xSemaphoreCreateBinary();

      void requestUpdate() {
        xSemaphoreGive(KKB::keyChange);
      }

      struct Key {
        String symbol = "aA";
        int width = 1;
        Key(const char *s, int w = 1) {
          symbol = s;
          width = w;
        }
      };

      std::vector<Key> keys = {
        Key("1!"), Key("2@"), Key("3#"), Key("4$"), Key("5%"), Key("6^"), Key("7&"), Key("8*"), Key("9("), Key("0)"),
        Key("qQ"), Key("wW"), Key("eE"), Key("rR"), Key("tT"), Key("yY"), Key("uU"), Key("iI"), Key("oO"), Key("pP"),
        Key("aA"), Key("sS"), Key("dD"), Key("fF"), Key("gG"), Key("hH"), Key("jJ"), Key("kK"), Key("lL"), Key(";:"),
        Key("zZ"), Key("xX"), Key("cC"), Key("vV"), Key("bB"), Key("nN"), Key("mM"), Key(",<"), Key(".>"), Key("/?"),
        Key("~`"), Key("-_"), Key("=+"), Key("  ", 4),                             Key("\'\""), Key("[{"), Key("]}"),
      };

      

      void drawKeys(String header) {
        canvas.clear(TFT_WHITE);
        char temp[2] = "A";
        int additionalOffset = 0;
        for(int i = 0; i < keys.size(); i++) {
          int x = offX+(blockSize+1)*(i%10) + additionalOffset;
          int y = offY+(blockSize+1)*(i/10);
          if (keys[i].width > 1) additionalOffset +=  (keys[i].width-1)*(blockSize+1);
          canvas.fillRoundRect(x, y, keys[i].width*blockSize + (keys[i].width-1), blockSize, 4, (activeKey == i)?TFT_NAVY:TFT_DARKGREEN);
          canvas.setTextDatum(textdatum_t::middle_center);
          canvas.setTextColor(TFT_WHITE);
          canvas.setFont(&fonts::AsciiFont8x16);
          // if (activeKey == i) canvas.drawRoundRect(x, y, keys[i].width*blockSize + (keys[i].width-1), blockSize, 4, TFT_GOLD);
          temp[0] = keys[i].symbol[shift];
          canvas.drawString(temp, x+((blockSize*keys[i].width)>>1), y+(blockSize>>1));
          
        } 

        textBox.clear(TFT_WHITE);
        textBox.setCursor(0, 0);
        textBox.setFont(&fonts::DejaVu18);
        textBox.setTextColor(TFT_DARKGRAY, TFT_WHITE);
        textBox.print(buffer);

        textBox.pushSprite(offX, offY-100);

        canvas.setTextDatum(textdatum_t::top_left);
        canvas.setCursor(0, 0);
        canvas.setFont(&fonts::DejaVu18);
        canvas.setTextColor(TFT_DARKGRAY, TFT_WHITE);
        canvas.print(header);
      }
    }

    String keyboard(String header = "Input", String def = "") {

      KKB::textBox.setColorDepth(16);
      KKB::textBox.setPsram(true);
      KKB::textBox.createSprite(210, 85);

      for(int i = 0; i < 256; i ++) {
        KKB::buffer[i] = 0;
      }

      KKB::endOfInput = false;
      
      KKB::requestUpdate();

      onKeyPress(JOY_LEFT, [](uint8_t k){
        KOS::KKB::activeKey--;
        KKB::requestUpdate();
      });

      onKeyPress(JOY_RIGHT, [](uint8_t k){
        KOS::KKB::activeKey++;
        KKB::requestUpdate();
      });

      onKeyPress(JOY_UP, [](uint8_t k){
        if(KOS::KKB::activeKey >= 43) KOS::KKB::activeKey+=3;
        KOS::KKB::activeKey -= 10;
        KKB::requestUpdate();
      });

      onKeyPress(JOY_DOWN, [](uint8_t k){
        if(KOS::KKB::activeKey >= 33) KOS::KKB::activeKey+=10-min(KOS::KKB::activeKey-33, 3);
        else KOS::KKB::activeKey += 10;
        KKB::requestUpdate();
      });

      onKeyPress(BTN_UP, [](uint8_t k) {
        KOS::KKB::shift = true;
        KOS::KKB::requestUpdate();
      });

      onKeyRelease(BTN_UP, [](uint8_t k) {
        KOS::KKB::shift = false;
        KOS::KKB::requestUpdate();
      });

      onKeyPress(JOY_CENTER, [](uint8_t k) {
        KOS::KKB::buffer[strlen(KOS::KKB::buffer)] = KOS::KKB::keys[KOS::KKB::activeKey].symbol[KOS::KKB::shift];
        KOS::KKB::requestUpdate();
      });

      onKeyPress(BTN_DOWN, [](uint8_t k) {
        KOS::KKB::buffer[strlen(KOS::KKB::buffer)-1] = 0;
        KOS::KKB::requestUpdate();
      });

      onKeyPress(BTN_BOOT, [](uint8_t k) {
        KOS::KKB::endOfInput = true;
        KOS::KKB::requestUpdate();
      });      

      while(true) {
        xSemaphoreTake(KKB::keyChange, portMAX_DELAY);
        if(KKB::endOfInput) break;
        KKB::drawKeys(header);
        canvas.pushSprite(0,0);
      }

      KKB::textBox.deleteSprite();

      return String(KKB::buffer);
    }
}
