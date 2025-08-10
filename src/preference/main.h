


namespace preference {
    #include "logo.h"

    TaskHandle_t mainProcess;
    // char buffer[128];

    SemaphoreHandle_t generalSemaphore = xSemaphoreCreateBinary();

    uint64_t mac_bin;

    String ssid;
    String password;

    String mac2String(byte ar[]) {
        String s;
        for (byte i = 0; i < 6; ++i)
        {
            char buf[3];
            sprintf(buf, "%02X", ar[i]); // J-M-L: slight modification, added the 0 in the format for padding 
            s += buf;
            if (i < 5) s += ':';
        }
        return s;
    }

    void showAboutDevice(void * arg) {
        // mac_bin = ESP.getEfuseMac();
        
        // String mac = mac2String((byte *) mac_bin);
        KUI::window = {
            KUI::Element{ELEMENT_TEXT, "Number of cores = "+String(ESP.getChipCores()), NULL, TFT_RED, NULL},
            KUI::Element{ELEMENT_TEXT, "CPU Frequency = "+String(ESP.getCpuFreqMHz()) + " Mhz", NULL, TFT_ORANGE, NULL},
            KUI::Element{ELEMENT_TEXT, "Chip model = " + String(ESP.getChipModel()), NULL, TFT_YELLOW, NULL},
            KUI::Element{ELEMENT_TEXT, "MAC = "+ WiFi.macAddress(), NULL, TFT_GREEN, NULL},
            KUI::Element{ELEMENT_TEXT, "ROM = "+String(ESP.getFlashChipSize()/1024.0/1024.0) + " MB", NULL, TFT_CYAN, NULL},
            KUI::Element{ELEMENT_TEXT, "RAM = "+String((ESP.getPsramSize()+ESP.getHeapSize())/1024.0/1024.0) + " MB", NULL, TFT_BLUE, NULL},
            KUI::Element{ELEMENT_TEXT, "Purple", NULL, TFT_PURPLE, NULL},
        };

        KUI::requestWindowUpdate();

        vTaskDelete(NULL);
    }

    void memTest(void * arg) {
        // KUI::terminateWindow();
        std::vector<KUI::Element> snapshot = KUI::window;

        uint8_t* p1 = (byte*) ps_malloc(1024*1024);
        uint8_t* p2 = (byte*) ps_malloc(1024*1024);

        uint64_t tmr = micros();

        for(uint32_t i = 0; i < 1024*1024; i++) {
            p2[i] = p1[i];
        }

        tmr = micros()-tmr;

        float speed = 1000000.0f / float(tmr);

        USBSerial.printf("SPIRAM speed: %f MB/s\n", speed);
        free(p1);
        free(p2);

        p1 = (byte*) ps_malloc(100000);
        p2 = (byte*) malloc(100000);
        tmr = micros();
        for(uint32_t i = 0; i < 100000; i++) {
            p2[i] = p1[i];
        }
        tmr = micros()-tmr;

        speed = 100000.0f / float(tmr);

        USBSerial.printf("PSRAM->SRAM speed: %f MB/s\n", speed);

        free(p1);
        free(p2);

        vTaskDelete(NULL);
    }

    void IRAM_ATTR aboutDevice() {
        xTaskCreate(
            showAboutDevice,
            "SHDevice task",
            4096,
            NULL,
            5,
            NULL
        );
    }

    struct tm timeinfo;

    bool mode = 0;

    void EEPROMTool(void *p) {
        KUI::activeElement = 0;
        KUI::window = {
            KUI::Element{ELEMENT_BUTTON, "Read", NULL, TFT_GREEN, [](){
                mode = 0; xSemaphoreGive(generalSemaphore);
            }},
            KUI::Element{ELEMENT_BUTTON, "Write", NULL, TFT_RED, []() {
                mode = 1; xSemaphoreGive(generalSemaphore);
            }}
        };
        KUI::requestWindowUpdate();

        xSemaphoreTake(generalSemaphore, portMAX_DELAY);

        if(mode == 0) {
            char buffer[4096];

            KOS::eeprom.readBlock(0, (uint8_t*) buffer, 4096);
            buffer[4095] = 0;

            KUI::window = {
                KUI::Element(ELEMENT_TEXT, String(buffer), NULL, TFT_CYAN, NULL)
            };

            KUI::requestWindowUpdate();
        }
        else {
            KUI::terminateWindow();
            String buffer = KOS::keyboard("Write in EEPROM");
            KOS::eeprom.writeBlock(0, (uint8_t*) buffer.c_str(), buffer.length()+1);

            
        }


        vTaskDelete(NULL);
    }

    uint16_t rainbow[] = {TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_CYAN, TFT_BLUE, TFT_PURPLE};

    void wifiMenu(void* p) {
        WiFi.mode(WIFI_STA);

        KUI::window={KUI::Element{ELEMENT_TEXT, "System is scanning Wi-Fi networks...", NULL, TFT_YELLOW, NULL}};
        KUI::initWindow();

        int n = WiFi.scanNetworks();
        

        KUI::window[0].text = "Done scanning! Found " + String(n) + " Wi-Fi networks: ";
        for(int i = 0; i < n; i++) {
            KUI::window.push_back(KUI::Element{ELEMENT_BUTTON, WiFi.SSID(i), NULL, rainbow[(i)%7], []() {
                xSemaphoreGive(generalSemaphore);

                ssid = WiFi.SSID(KUI::activeElement-1);
            }});
        }

        KUI::requestWindowUpdate();

        xSemaphoreTake(generalSemaphore, portMAX_DELAY);

        KUI::terminateWindow();

        password = KOS::keyboard("Password for "+ssid);

        KUI::window = {KUI::Element(ELEMENT_TEXT, "System is connecting to "+ssid, NULL, TFT_PINK, NULL)};
        
        KUI::initWindow();

        WiFi.begin(ssid.c_str(), password.c_str());

        while(WiFi.status() != WL_CONNECTED) {
            vTaskDelay(500);
            USBSerial.println("Wait...");
        }
        configTime(3600*3, 0, "ntp.ix.ru");

        KOS::saveWiFiCredentials(ssid, password);

        // getLocalTime(&timeinfo);

        KUI::window.push_back(KUI::Element(ELEMENT_TEXT, "Connecteda succesfuly", NULL, TFT_GOLD, NULL));
        KUI::window.push_back(KUI::Element(ELEMENT_TEXT, "Push button to get time", NULL, TFT_BLUE, NULL));
        KUI::window.push_back(KUI::Element(ELEMENT_BUTTON, "Update datetime", NULL, TFT_GREEN, []() {
            xSemaphoreGive(generalSemaphore);
        }));


        KUI::requestWindowUpdate();

        while(true) {
            xSemaphoreTake(generalSemaphore, portMAX_DELAY);
            getLocalTime(&timeinfo);
            KUI::window[2].text = String(asctime(&timeinfo));
            KUI::requestWindowUpdate();
        }


        vTaskDelete(NULL);
    }

    void saveAutoSleep(void * arg) {
        File d = SPIFFS.open("/autoSleep.conf");
        d.println(KOS::autoSleep::enable ? KOS::autoSleep::timeout : 0xffffffffUL);
        d.close();
    }

    #ifdef KOROBOCHKA3 
    int light_pin = 45;
    #else  
    int light_pin = 18;
    #endif

    void main(void* p) {
        USBSerial.println("Preference task started");

        KUI::window = {
            KUI::Element{ELEMENT_BUTTON, "Wi-Fi",        NULL, TFT_GREEN, [](){
                KUI::terminateWindow();
                xTaskCreatePinnedToCore(
                    wifiMenu,
                    "Wifi setup task",
                    16384,
                    NULL,
                    1,
                    NULL, 
                    1
                );
            }},
            // KUI::Element{ELEMENT_BUTTON, "Bluetooth",    NULL, TFT_GREEN, NULL},
            // KUI::Element{ELEMENT_BUTTON, "Timezone",     NULL, TFT_GREEN, NULL},
            // KUI::Element{ELEMENT_BUTTON, "Temperature",  NULL, TFT_GREEN, NULL},
            KUI::Element{ELEMENT_SWITCH, "Auto OFF",  NULL, TFT_ORANGE, [](){
                
            }, &fonts::DejaVu18, &KOS::autoSleep::enable},
            KUI::Element{ELEMENT_BUTTON, "WeChat nickname", NULL, TFT_GOLD, [](){
                xTaskCreate(WeChat::changeName, "wCht ch name", 4096, NULL, 5, NULL);
            }},
            KUI::Element{ELEMENT_BUTTON, "MemTest", NULL, TFT_BLUE, [](){
                xTaskCreatePinnedToCore(memTest, "memtest", 16384, NULL, 10, NULL, 1);
            }},
            KUI::Element{ELEMENT_BUTTON, "About device", NULL, TFT_GREEN, aboutDevice},
            KUI::Element{ELEMENT_BUTTON, "EEPROM", NULL, TFT_PINK, []() {
                xTaskCreatePinnedToCore(
                    EEPROMTool,
                    "eepromTool",
                    16384,
                    NULL,
                    1,
                    NULL, 
                    1
                );
            }},
            KUI::Element{ELEMENT_BUTTON, "Flashlight", NULL, TFT_CYAN, [](){
                
                pinMode(light_pin, OUTPUT);
                KUI::activeElement = 0;
                KUI::window = {
                    KUI::Element(ELEMENT_BUTTON, "Power ON",  NULL, TFT_GREEN, [](){digitalWrite(light_pin, HIGH);}),
                    KUI::Element(ELEMENT_BUTTON, "Power OFF", NULL, TFT_RED,   [](){digitalWrite(light_pin, LOW);}),
                    KUI::Element(ELEMENT_BUTTON, "Toggle",    NULL, TFT_BLUE,  [](){digitalWrite(light_pin, !digitalRead(light_pin));}),
                };
                KUI::requestWindowUpdate();
                
            }},
            KUI::Element{ELEMENT_BUTTON, "Sound check", NULL, TFT_CYAN, [](){
                
                pinMode(1, OUTPUT);
                KUI::activeElement = 0;

                // ledcSetup(3, 400, 16);

                ledcAttach(1, 340, 10);

                // ledc_timer_config_t ledc_timer = {
                //     .speed_mode       = LEDC_LOW_SPEED_MODE,
                //     .duty_resolution  = LEDC_TIMER_12_BIT,
                //     .timer_num        = LEDC_TIMER_0,
                //     .freq_hz          = 400,  // Set output frequency at 0.4 kHz
                //     .clk_cfg          = LEDC_AUTO_CLK
                // };
                // ledc_timer_config(&ledc_timer);

                // ledc_channel_config_t ledc_channel = {
                //     .speed_mode     = LEDC_LOW_SPEED_MODE,
                //     .channel        = LEDC_CHANNEL_0,
                //     .timer_sel      = LEDC_TIMER_0,
                //     .intr_type      = LEDC_INTR_DISABLE,
                //     .gpio_num       = 1,
                //     .duty           = 50, // Set duty to 50%
                //     .hpoint         = 0
                // };
                // ledc_channel_config(&ledc_channel);

                

                KUI::window = {
                    KUI::Element(ELEMENT_BUTTON, "200Hz",  NULL, TFT_GREEN, [](){ledcWriteTone(1, 200);}),
                    KUI::Element(ELEMENT_BUTTON, "500Hz", NULL, TFT_RED,   [](){ledcWriteTone(1, 500);}),
                    KUI::Element(ELEMENT_BUTTON, "1000Hz",    NULL, TFT_BLUE,  [](){ledcWriteTone(1, 1000);}),
                };
                KUI::requestWindowUpdate();
                
            }},
        };

        KUI::initWindow();

        xSemaphoreTake(generalSemaphore, 1);        

        vTaskDelete(NULL);
    }

    void init() {
        xTaskCreatePinnedToCore(
            main,
            "Preference task",
            16384,
            NULL,
            1,
            &mainProcess, 
            1
        );
    }
}