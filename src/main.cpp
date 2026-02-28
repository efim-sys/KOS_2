#include "KOS/KOS.h"
#include "KUI/KUI.h"

// Include Apps here:

#include "doodle_jump/main.h"
#include "infinite_bunner/main.h"
#include "WeChat/main.h"
#include "preference/main.h"
#include "korobochka_commander/main.h"
#include "zmeyka/main.h"
#include "StopWatch/main.h"
#include "doom/main.h"
#include "robot_control/main.h"
#include "flappy_bird/main.h"
#include "watch/main.h"
#include "cardReader/main.h"
#include "remote/main.h"
#include "launcher/main.h"

#include "calendar/main.h"
#include "clock/main.h"




// Stop including apps here

struct Application {
  String name;

  // const lgfx::rgb565_t* logo;
  // int logoH = 80;
  // int logoW = 80;

  void (*function)();

  uint16_t color;

  // Application(String _name, void (*_function)(), const lgfx::rgb565_t* _logo) {
  //   function = _function;
  //   logo = _logo;
  //   name = _name;
  // }
};

std::vector <Application> apps;

SemaphoreHandle_t appSelected = xSemaphoreCreateBinary();

TaskHandle_t timekeep_task;

void setup() {
  KOS::initI2C();
  KOS::initDisplay();
  KOS::initButtons();
  KOS::initSPIFFS();

  KOS::autoSleep::init();
  
  // KOS::initSD();

  #ifndef DEBUG_MODE
  
    USB.manufacturerName("Efim Sysoev's");
    #ifdef KOROBOCHKA3
    USB.productName("Korobochka 3");
    #else
    USB.productName("Korobochka NEON");
    #endif
    
    USB.begin();

    

  #endif

  USBSerial.setTxBufferSize(8192);

  USBSerial.begin();

  if(digitalRead(16)) {
    pinMode(1, OUTPUT);
    pinMode(10, OUTPUT);

    while(true) {
      USBSerial.println(digitalRead(18)*100);
      digitalWrite(1, !digitalRead(18));
      digitalWrite(10, !digitalRead(18));
      delayMicroseconds(10);
    }

  } 

  // pinMode(0, INPUT_PULLUP);

  // while(digitalRead(0)) USBSerial.println(millis());

  canvas.clear(TFT_WHITE);
  


  apps = {
    {"Launcher", &launcher::init, TFT_RED},
    {"Flappy bird", &FlappyBird::init, TFT_GREEN},
    {"Doodle jump", &doodle_jump::init, TFT_WHITE}, 
    #ifdef IPS169
    {"Bunner",      &infinite_bunner::init, TFT_GREENYELLOW}, 
    #endif
    // Application{"Battleship",      &BattleShip::init, infinite_bunner::logo}, 
    {"Settings", &preference::init, TFT_SKYBLUE},
    {"Korobka commander", &commander_init, TFT_SKYBLUE},
    {"Zmeyka", &zmeyka::init, TFT_LIGHTGRAY},
    // {"StopWatch", &stopWatch::init, TFT_ORANGE},
    // Application{"Video", &video::init, NULL},
    // {"Calendar", &calendar::init, TFT_ORANGE},
    // {"BLUETOOTH moudse", &BLE_MOUSE::init, TFT_PINK},
    {"WeChat", &WeChat::init, TFT_PURPLE},
    {"DooM", &doom::init, TFT_RED},
    // {"Robot", &robot_control::init, NULL},
    {"CardReader mode", &cardReader::init, TFT_SKYBLUE},
    {"TV remote", &TVremote::init, TFT_MAROON},
    
  };
  if(KOS::extRTC_IN) {  
    apps.insert(apps.begin()+1, 
      {"Clock", &watch::init, TFT_ORANGE}
    );
  }
  

  for(Application i : apps) {
    KUI::window.push_back(KUI::Element(ELEMENT_BUTTON, i.name, NULL, i.color, [](){xSemaphoreGiveFromISR(appSelected, NULL);}));
  }

  KUI::window.push_back(KUI::Element(ELEMENT_TEXT, 
    #ifdef KOROBOCHKA3
    "Device: Korobochka 3\n(04.2025)"
    #else
    "Device: Korobochka 07.2024 (prototype)"
    #endif
    , NULL, TFT_WHITE, NULL));
  KUI::window.push_back(KUI::Element(ELEMENT_TEXT, "Firmware compilation date: " + String(__DATE__), NULL, TFT_WHITE, NULL));

  KUI::window.push_back(KUI::Element(ELEMENT_TEXT, "Battery voltage: " + String(KOS::getBatteryVoltage()), NULL, TFT_WHITE, NULL));

  

  xSemaphoreTake(appSelected, 1);

  KUI::initWindow();
  
}


void loop() {
  

  xSemaphoreTake(appSelected, portMAX_DELAY);
  USBSerial.println("App selected!");
  int choosedApp;

  choosedApp = KUI::activeElement;

  KUI::terminateWindow();
  apps[choosedApp].function();
}