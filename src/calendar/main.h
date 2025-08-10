namespace calendar {
    struct Lesson {
        uint startH;
        uint startM;
        
        uint endH;
        uint endM;
        
        String name;

        uint16_t color;
    };

    SemaphoreHandle_t sm = xSemaphoreCreateBinary();

    String topText;

    void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
    {
        Serial.println("Connected to AP!");
        KOS::initRTC();
        topText = KOS::getTimeString();
        if(KUI::window[0].text.startsWith("Connecting")) {
            KUI::window[0].text = topText;
            KUI::requestWindowUpdate();
        }
    }

    void main(void * arg) {

        
        
        if(!KOS::WiFiConnected()) {
            KOS::initWiFi();
            WiFi.onEvent(WiFiStationConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
        }

        std::vector<std::vector<Lesson>> schedule = {
            {   
                Lesson{8, 15, 9, 0,  "Razgovor", TFT_YELLOW},
                Lesson{9, 5,  9, 50, "English",  TFT_BLUE},
                Lesson{10,0,  10,45, "English",  TFT_BLUE},
                Lesson{10, 55,11,40,  "Literature",  TFT_ORANGE},
                Lesson{11,50,12,35, "Literature",  TFT_ORANGE},
                Lesson{12,45, 13,30, "Informatics",  TFT_WHITE},
                Lesson{13,55,14,40, "Informatics",  TFT_WHITE},
                Lesson{15,05,15,50, "Phisics (1gr)",  TFT_GREEN},
                Lesson{16,0,16,45, "Phisics (1gr)",  TFT_GREEN},
                
            },
            {
                Lesson{8, 30, 9, 15, "Phisics",  TFT_GREEN},
                Lesson{9, 25, 10, 10, "Phisics",  TFT_GREEN},
                Lesson{10, 20, 11, 05, "Chemistry",  TFT_MAGENTA},
                Lesson{11, 15, 12, 00, "Chemistry",  TFT_MAGENTA},
                Lesson{12,10,12,55, "Geometry",  TFT_PINK},
                Lesson{13,20,14,05, "Algebra",  TFT_CYAN},
                Lesson{14, 30, 15, 15, "Russian",  TFT_RED},
            },
            {
                Lesson{8, 30, 9, 15, "Empty",  TFT_SKYBLUE},
                Lesson{9, 25, 10, 10, "Empty",  TFT_SKYBLUE},
                Lesson{10, 20, 11, 05, "Russian",  TFT_RED},
                Lesson{11, 15, 12, 00, "Russian",  TFT_RED},
                Lesson{12,10,12,55, "Algebra",  TFT_CYAN},
                Lesson{13,20,14,05, "Algebra",  TFT_CYAN},
                Lesson{14, 30, 15, 15, "Phisics",  TFT_GREEN},
                Lesson{15, 20, 16, 05, "Phisics",  TFT_GREEN},
            },
            {
                Lesson{8, 30, 9, 15, "Empty",  TFT_SKYBLUE},
                Lesson{9, 25, 10, 10, "Informatics",  TFT_WHITE},
                Lesson{10, 20, 11, 05, "Informatics",  TFT_WHITE},
                Lesson{11, 15, 12, 00, "Statistics",  TFT_SILVER},
                Lesson{12,10,12,55, "Geometry",  TFT_PINK},
                Lesson{13,20,14,05, "Geometry",  TFT_PINK},
                Lesson{14, 30, 15, 15, "English",  TFT_BLUE},
                Lesson{15, 20, 16, 05, "Literature",  TFT_ORANGE},
            },
            {
                Lesson{8, 30, 9, 15, "Phisics",  TFT_GREEN},
                Lesson{9, 25, 10, 10, "Phisics",  TFT_GREEN},
                Lesson{10, 20, 11, 05, "Algebra",  TFT_CYAN},
                Lesson{11, 15, 12, 00, "Algebra",  TFT_CYAN},
                Lesson{12,10,12,55, "Fist culture",  TFT_PURPLE},
                Lesson{13,20,14,05, "Fist culture",  TFT_PURPLE},
            },
        };

        String weekdays[] = {"Monday (5-9L)", "Tuesday (7L)", "Wednesday (6L)", "Thursday (5-8L)", "Friday (6L)"};
        uint16_t colors[] = {TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_CYAN};

        String ssid;
        KOS::getWiFiCredentials(&ssid, NULL);

        topText = "Connecting to " + ssid;
        if(KOS::WiFiConnected()) topText = KOS::getTimeString();

        while(true) {
            KUI::window = {KUI::Element(ELEMENT_TEXT, topText, NULL, TFT_WHITE, NULL)};

            

            xSemaphoreTake(sm, 1);

            for(int i = 0; i < 5; i++) {
                KUI::window.push_back(KUI::Element(ELEMENT_BUTTON, weekdays[i], NULL, colors[i], [](){
                    xSemaphoreGive(sm);
                }));
            }

            KUI::initWindow();

            xSemaphoreTake(sm, portMAX_DELAY);

            int weekday = KUI::activeElement - 1;

            KUI::window = {KUI::Element(ELEMENT_BUTTON, "Back", NULL, TFT_ORANGE, [](){
                    xSemaphoreGive(sm);
                })};

            char txt[50];

            for(int i = 0; i < schedule[weekday].size(); i++) {
                

                snprintf(txt, 50, "%02d:%02d-%02d:%02d %s", 
                    schedule[weekday][i].startH, 
                    schedule[weekday][i].startM, 
                    schedule[weekday][i].endH, 
                    schedule[weekday][i].endM, 
                    schedule[weekday][i].name.c_str());
                KUI::window.push_back(KUI::Element(ELEMENT_TEXT, String(txt), NULL, schedule[weekday][i].color, NULL));
            }

            KUI::activeElement = 0;
            KUI::scrollY = 0;

            KUI::requestWindowUpdate();

            xSemaphoreTake(sm, portMAX_DELAY);
            
            KUI::terminateWindow();

        }


    }

    void init() {
        xTaskCreate(main, "Calendar task", 8192, NULL, 3, NULL);
    }
}