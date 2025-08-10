namespace watch {
    SemaphoreHandle_t generalSemaphore = xSemaphoreCreateBinary();
    bool sync_in_process = false;

    void syncTime(void * p) {
        KOS::initWiFi();

        while(WiFi.status() != WL_CONNECTED) vTaskDelay(100);

        configTime(3600*3, 0, "ntp.ix.ru");

        struct tm timeinfo;

        getLocalTime(&timeinfo);

        KOS::extRTC.setMonth(timeinfo.tm_mon);
        KOS::extRTC.setYear(timeinfo.tm_year-100);
        KOS::extRTC.setHour(timeinfo.tm_hour);
        KOS::extRTC.setMinute(timeinfo.tm_min);
        KOS::extRTC.setSecond(timeinfo.tm_sec);
        KOS::extRTC.setDate(timeinfo.tm_mday);
        KOS::extRTC.setDoW(timeinfo.tm_wday);

        WiFi.disconnect(true);
        WiFi.setSleep(true);

        sync_in_process = false;

        KUI::window[2].text = "Sync time on-line";
        KUI::requestWindowUpdate();

        vTaskDelete(NULL);
        
    }

    void main(void * p) {
        KUI::window = {
            KUI::Element(ELEMENT_TEXT, "DD-MM-YYYY DoW", NULL, TFT_GREEN, NULL),
            KUI::Element(ELEMENT_TEXT, "HH-MM-SS", NULL, TFT_GREEN, NULL),
            KUI::Element(ELEMENT_BUTTON, "Sync time on-line", NULL, TFT_ORANGE, [](){
                sync_in_process=true;
                xTaskCreate(
                    syncTime,
                    "time syncing",
                    32768,
                    NULL,
                    5,
                    NULL);
                KUI::window[2].text = "Syncing...";
                KUI::requestWindowUpdate();
            }),
            KUI::Element(ELEMENT_SWITCH, "Auto powerOFF", NULL, TFT_ORANGE, [](){
                KOS::autoSleep::enable = !KOS::autoSleep::enable;
            }),
            KUI::Element(ELEMENT_BUTTON, "Sleep", NULL, TFT_RED, [](){
                KOS::autoSleep::sleepNow();
            }),
            
            
        };

        KUI::window[3].virtualSwitch = KOS::autoSleep::enable;

        KUI::initWindow();

        bool e = false;

        const char dowStr[7][12] ={
            "Sunday",
            "Monday",
            "Tuesday",
            "Wednesday",
            "Thursday",
            "Friday",
            "Saturday",
            
        };

        while(true) {
            char buffer[32];
            snprintf(buffer, 32, "\n\n%02d.%02d.%04d %s", KOS::extRTC.getDate(), KOS::extRTC.getMonth(e)+1, KOS::extRTC.getYear()+2000, dowStr[KOS::extRTC.getDoW()]);

            KUI::window[0].text = String(buffer);
            // String time = String(KOS::extRTC.getHour(e, e)).
            snprintf(buffer, 32, "%02d:%02d:%02d", KOS::extRTC.getHour(e, e), KOS::extRTC.getMinute(), KOS::extRTC.getSecond());

            KUI::window[1].text = String(buffer);

            KUI::requestWindowUpdate();

            vTaskDelay(1000);
        }
        
    }

    void init() {
        xTaskCreatePinnedToCore(
        main,
        "Watch",
        8192,
        NULL,
        3,
        NULL,
        1);
    }
}