#pragma once

#include <Arduino.h>
#include "defines.h"

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

#include "output.h"

#include "display.h"

enum {
  BTN_UP = 0,
  BTN_DOWN,
  BTN_BOOT,
  JOY_UP,
  JOY_DOWN,
  JOY_LEFT,
  JOY_RIGHT,
  JOY_CENTER
};






struct Button{
    uint8_t pin;
    uint8_t id;
    uint32_t lastInterrupt = 0;

    bool inverted = false;

    bool lastValue;
    const char *name = "Unnamed button";

    void (*onKeyPress)(uint8_t keyID) = nullptr;
    void (*onKeyRelease)(uint8_t keyID) = nullptr;

    void init(uint8_t _id, uint8_t _pin, const char *_name, void (*intHandler)(void*), bool _inverted = false) {
        id = _id;
        pin = _pin;
        inverted = _inverted;
        pinMode(pin, inverted ? INPUT_PULLUP : INPUT_PULLDOWN);
        lastValue = digitalRead(pin);
        name = _name;
        attachInterruptArg(pin, intHandler, &id, CHANGE);
        
    }
};

extern Button buttons[8];


namespace KOS{   
    float getBatteryVoltage();
    float getBattery();

    extern I2C_eeprom eeprom;
    extern DS3231 extRTC;
    extern bool extRTC_IN;

    extern std::vector<uint8_t> I2C_dev;

    std::vector<uint8_t> scanI2C(TwoWire &wire = Wire);
    

    namespace autoSleep {
      extern bool enable;
      extern int32_t timeout;

      extern uint64_t lastButtonPress;

      void sleepNow();

      void init();
    }

    void intHandler(void* arg);

    void playSound(fs::FS *fs, String filename);

    void initButtons();

    void initSPIFFS();

    void initRTC();

    void initI2C();

    void initDisplay();

    String getTimeString();




    uint16_t *rotateImageRight(uint16_t *from, int w, int h);

    void flipImageV(uint16_t *img, int w, int h);

    void flipImageV(uint16_t *img, uint16_t **destination, int w, int h);

    void flipImageH(uint16_t *img, int w, int h);

    void flipImageH(uint16_t *img, uint16_t **destination, int w, int h);

    uint16_t *readImageBmp(fs::FS &fs, const char *filename, uint32_t *width = NULL, uint32_t *height = NULL);

    void readImageBmp(fs::FS &fs, const char *filename, uint32_t *width, uint32_t *height, uint16_t *addr);

    int saveFramebuffer(uint16_t *framebuffer, uint32_t w, uint32_t h, const char* filename);


    

    void initWiFi(String ssid, String password);

    void initWiFi(fs::FS &fs = SPIFFS,  String filename = "/wifi.conf");

    bool WiFiConnected();

    void saveWiFiCredentials(String ssid, String pass, fs::FS &fs = SPIFFS,  String filename = "/wifi.conf");

    void getWiFiCredentials(String* ssid, String* pass, fs::FS &fs = SPIFFS,  String filename = "/wifi.conf");

    extern bool sdInstalled;

    bool initSD(bool skipInsertCheck = false);



    
    extern TaskHandle_t soundPlayTask;

    void playSound(fs::FS *fs, String filename);



    // Для привязки функции к событию, укажите функцию, для отвязки не указывайте

    void onKeyPress(uint8_t key, void (*function)(uint8_t keyID) = nullptr);

    void onKeyRelease(uint8_t key, void (*function)(uint8_t keyID) = nullptr);

    void onKeyAll(uint8_t key, void (*function)(uint8_t keyID) = nullptr);

    void detachAllKeys();

    bool copyFile(fs::FS &sourceFS, fs::FS &destFS, const char* sourcePath, const char* destPath);

    

    String keyboard(String header = "Input", String def = "");
}
