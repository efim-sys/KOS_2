#include "KOS.h"

Button buttons[8];

namespace KOS {
    #ifndef KOROBOCHKA3
    float getBatteryVoltage()
    {
        int b = analogRead(4);
        return 1.1 * 3.1 * 2.0 * (b / 4095.0);
    }
    #else

    const int debounceMs = 20;

    void intHandler(void* arg) {
        uint8_t thisID = *((uint8_t*) arg);

        if (millis()-buttons[thisID].lastInterrupt < debounceMs) return;
        if (buttons[thisID].lastValue == buttons[thisID].inverted) {
            if(buttons[thisID].onKeyPress != nullptr) buttons[thisID].onKeyPress(thisID);
            KOS::autoSleep::lastButtonPress = millis();
            buttons[thisID].lastValue = !buttons[thisID].inverted;
        }
        else {
            if(buttons[thisID].onKeyRelease != nullptr) buttons[thisID].onKeyRelease(thisID);
            KOS::autoSleep::lastButtonPress = millis();
            buttons[thisID].lastValue = buttons[thisID].inverted;
        }
        
        buttons[thisID].lastInterrupt = millis();
    }

    bool firstMeasuring = true;

    float getBatteryVoltage()
    {
        pinMode(46, OUTPUT);
        digitalWrite(46, HIGH);
        if (firstMeasuring)
        {
            delay(10);
            firstMeasuring = false;
        }
        digitalWrite(46, 0);
        delayMicroseconds(25);
        int b = analogRead(4);
        digitalWrite(46, HIGH);
        return 1.1 * 3.1 * 2.0 * (b / 4095.0);
    }
    #endif

    float getBattery()
    {
        const float highV = 4.2;
        const float lowV = 3.0;

        return (getBatteryVoltage() - lowV) / (highV - lowV);
    }




    I2C_eeprom eeprom(0x50, I2C_DEVICESIZE_24LC32, &Wire);
    DS3231 extRTC;

    bool extRTC_IN = false;

    std::vector<uint8_t> I2C_dev;

    std::vector<uint8_t> scanI2C(TwoWire &wire) {
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

    void initI2C() {
      Wire.begin(2, 17);

      eeprom.begin();

      I2C_dev = scanI2C();
      for(uint8_t i : I2C_dev) {
        if(i == 0x68) extRTC_IN = true;
      }
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
        xTaskCreatePinnedToCore(routine, "AutoSleep", 2048, NULL, 5, NULL, 0);
      }
    }











    void initButtons() {
        buttons[BTN_UP].    init(BTN_UP,      8,  (char *) "Button UP",       intHandler);
        buttons[BTN_DOWN].  init(BTN_DOWN,    9,  (char *) "Button DOWN",     intHandler);
        buttons[BTN_BOOT].  init(BTN_BOOT,    0,  (char *) "Button BOOT",     intHandler, true);
        buttons[JOY_UP].    init(JOY_UP,      5,  (char *) "JoyStick UP",     intHandler);
        buttons[JOY_DOWN].  init(JOY_DOWN,    7,  (char *) "JoyStick DOWN",   intHandler);
        buttons[JOY_LEFT].  init(JOY_LEFT,    15, (char *) "JoyStick LEFT",   intHandler);
        buttons[JOY_RIGHT]. init(JOY_RIGHT,   6,  (char *) "JoyStick RIGHT",  intHandler);
        buttons[JOY_CENTER].init(JOY_CENTER,  16, (char *) "JoyStick CENTER", intHandler);
    }

    void initSPIFFS() {
      SPIFFS.begin();
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















    bool sdInstalled = false;

    bool initSD(bool skipInsertCheck) {
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










    void onKeyPress(uint8_t key, void (*function)(uint8_t keyID)) {
        buttons[key].onKeyPress = function;
    }

    void onKeyRelease(uint8_t key, void (*function)(uint8_t keyID)) {
        buttons[key].onKeyRelease = function;
    }

    void onKeyAll(uint8_t key, void (*function)(uint8_t keyID)) {
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

    String keyboard(String header, String def) {

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