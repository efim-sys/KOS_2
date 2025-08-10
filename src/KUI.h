#include <vector>   

#define ELEMENT_BUTTON 0
#define ELEMENT_TEXT   1
#define ELEMENT_IMAGE  2
#define ELEMENT_SWITCH 3  
#define ELEMENT_SCROLLER 4

namespace KUI {

    const uint8_t button_arrow_img [] = {
        0xe0, 0x38, 0x00, 0xf0, 0x3c, 0x00, 0x78, 0x1e, 0x00, 0x3c, 0x0f, 0x00, 0x1e, 0x07, 0x80, 0x0f, 
        0x03, 0xc0, 0x07, 0x81, 0xe0, 0x03, 0x80, 0xe0, 0x07, 0x81, 0xe0, 0x0f, 0x03, 0xc0, 0x1e, 0x07, 
        0x80, 0x3c, 0x0f, 0x00, 0x78, 0x1e, 0x00, 0xf0, 0x3c, 0x00, 0xe0, 0x38, 0x00, 0xc0, 0x30, 0x00
    };

    // 'battery_android_0_24dp_5F6368_FILL0_wght400_GRAD0_opsz24 (1)', 24x24px
    const unsigned char bat_icon_battery_android_0_24dp_5F6368_FILL0_wght400_GRAD0_opsz24__1_ [] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 
        0xff, 0xe0, 0x3f, 0xff, 0xf0, 0x30, 0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x00, 
        0x3c, 0x30, 0x00, 0x3c, 0x30, 0x00, 0x3c, 0x30, 0x00, 0x3c, 0x30, 0x00, 0x30, 0x30, 0x00, 0x30, 
        0x30, 0x00, 0x30, 0x3f, 0xff, 0xf0, 0x1f, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    // 'battery_android_2_24dp_5F6368_FILL0_wght400_GRAD0_opsz24', 24x24px
    const unsigned char bat_icon_battery_android_2_24dp_5F6368_FILL0_wght400_GRAD0_opsz24 [] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 
        0xff, 0xe0, 0x3f, 0xff, 0xf0, 0x3f, 0x00, 0x30, 0x3f, 0x00, 0x30, 0x3f, 0x00, 0x30, 0x3f, 0x00, 
        0x3c, 0x3f, 0x00, 0x3c, 0x3f, 0x00, 0x3c, 0x3f, 0x00, 0x3c, 0x3f, 0x00, 0x30, 0x3f, 0x00, 0x30, 
        0x3f, 0x00, 0x30, 0x3f, 0xff, 0xf0, 0x1f, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    // 'battery_android_4_24dp_5F6368_FILL0_wght400_GRAD0_opsz24', 24x24px
    const unsigned char bat_icon_battery_android_4_24dp_5F6368_FILL0_wght400_GRAD0_opsz24 [] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 
        0xff, 0xe0, 0x3f, 0xff, 0xf0, 0x3f, 0xf0, 0x30, 0x3f, 0xf0, 0x30, 0x3f, 0xf0, 0x30, 0x3f, 0xf0, 
        0x3c, 0x3f, 0xf0, 0x3c, 0x3f, 0xf0, 0x3c, 0x3f, 0xf0, 0x3c, 0x3f, 0xf0, 0x30, 0x3f, 0xf0, 0x30, 
        0x3f, 0xf0, 0x30, 0x3f, 0xff, 0xf0, 0x1f, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    // 'battery_android_6_24dp_5F6368_FILL0_wght400_GRAD0_opsz24', 24x24px
    const unsigned char bat_icon_battery_android_6_24dp_5F6368_FILL0_wght400_GRAD0_opsz24 [] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 
        0xff, 0xe0, 0x3f, 0xff, 0xf0, 0x3f, 0xff, 0x30, 0x3f, 0xff, 0x30, 0x3f, 0xff, 0x30, 0x3f, 0xff, 
        0x3c, 0x3f, 0xff, 0x3c, 0x3f, 0xff, 0x3c, 0x3f, 0xff, 0x3c, 0x3f, 0xff, 0x30, 0x3f, 0xff, 0x30, 
        0x3f, 0xff, 0x30, 0x3f, 0xff, 0xf0, 0x1f, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    // 'battery_android_full_24dp_5F6368_FILL0_wght400_GRAD0_opsz24', 24x24px
    const unsigned char bat_icon_battery_android_full_24dp_5F6368_FILL0_wght400_GRAD0_opsz24 [] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 
        0xff, 0xe0, 0x3f, 0xff, 0xf0, 0x3f, 0xff, 0xf0, 0x3f, 0xff, 0xf0, 0x3f, 0xff, 0xf0, 0x3f, 0xff, 
        0xfc, 0x3f, 0xff, 0xfc, 0x3f, 0xff, 0xfc, 0x3f, 0xff, 0xfc, 0x3f, 0xff, 0xf0, 0x3f, 0xff, 0xf0, 
        0x3f, 0xff, 0xf0, 0x3f, 0xff, 0xf0, 0x1f, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    // Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 480)
    const int bat_icons_LEN = 5;
    const unsigned char* bat_icons[5] = {
        bat_icon_battery_android_0_24dp_5F6368_FILL0_wght400_GRAD0_opsz24__1_,
        bat_icon_battery_android_2_24dp_5F6368_FILL0_wght400_GRAD0_opsz24,
        bat_icon_battery_android_4_24dp_5F6368_FILL0_wght400_GRAD0_opsz24,
        bat_icon_battery_android_6_24dp_5F6368_FILL0_wght400_GRAD0_opsz24,
        bat_icon_battery_android_full_24dp_5F6368_FILL0_wght400_GRAD0_opsz24
    };
    uint16_t bat_colors[] = {TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREENYELLOW, TFT_GREEN};
    // 'wifi_off_24dp_5F6368_FILL0_wght400_GRAD0_opsz24', 24x24px
    const unsigned char wifi_icon_wifi_off_24dp_5F6368_FILL0_wght400_GRAD0_opsz24 [] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x30, 0x7f, 0x00, 0x18, 
        0xff, 0xe0, 0x1c, 0x7f, 0xf8, 0x3e, 0x00, 0xfc, 0x7b, 0x00, 0x1e, 0x71, 0x80, 0x0e, 0x00, 0xc2, 
        0x00, 0x01, 0xe1, 0x80, 0x07, 0xf0, 0xe0, 0x07, 0x98, 0x60, 0x02, 0x0c, 0x00, 0x00, 0x06, 0x00, 
        0x00, 0x1b, 0x00, 0x00, 0x3d, 0x80, 0x00, 0x3c, 0xc0, 0x00, 0x3c, 0x60, 0x00, 0x18, 0x30, 0x00, 
        0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    // 'wifi_24dp_5F6368_FILL0_wght400_GRAD0_opsz24', 24x24px
    const unsigned char wifi_icon_wifi_24dp_5F6368_FILL0_wght400_GRAD0_opsz24 [] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x07, 
        0xff, 0xe0, 0x1f, 0xff, 0xf8, 0x3f, 0x00, 0xfc, 0x78, 0x00, 0x1e, 0x70, 0x00, 0x0e, 0x00, 0x7e, 
        0x00, 0x01, 0xff, 0x80, 0x07, 0xff, 0xe0, 0x07, 0x81, 0xe0, 0x02, 0x00, 0x40, 0x00, 0x00, 0x00, 
        0x00, 0x18, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x18, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    // Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 192)
    const int wifi_icon_allArray_LEN = 2;
    const unsigned char* wifi_icon_allArray[2] = {
        wifi_icon_wifi_off_24dp_5F6368_FILL0_wght400_GRAD0_opsz24,
        wifi_icon_wifi_24dp_5F6368_FILL0_wght400_GRAD0_opsz24
    };

    uint16_t wifi_colors[] = {TFT_LIGHTGRAY, TFT_BLUE};

    struct Element {
        int8_t type = 0;

        String text;
        uint16_t* image;
        uint16_t color = TFT_MAGENTA;

        const lgfx::v1::GFXfont* font = &fonts::DejaVu18;

        void (*onClick)() = NULL;

        bool virtualSwitch = false;

        bool *switchPointer = NULL;

        Element(int8_t _type, String _text, uint16_t* _image, uint16_t _color, void (*_onClick)(), const lgfx::v1::GFXfont* _font = &fonts::DejaVu18, bool* swPtr = NULL ) {
            type = _type;
            text = _text;
            image = _image;
            color = _color;
            font = _font;
            onClick = _onClick;
            switchPointer = swPtr;
            if(switchPointer!=NULL) virtualSwitch = *switchPointer;
        }
    };

    std::vector<Element> window;

    int activeElement = 0;

    int allHeight = 0;

    int scrollY = 0;

    int scrollVelocity = 0;

    int scrollVelocityConst = 6;

    SemaphoreHandle_t updateNeeded;

    SemaphoreHandle_t clickMade;

    void requestWindowUpdate() {
        xSemaphoreGiveFromISR(KUI::updateNeeded, NULL);
    }

    const int headerHeight = 30;

    int drawHeader(int y) {
        char buffer[32] = "KOS";

        int centerX = canvas.width()/2;

        if(KOS::extRTC_IN) {
            bool e;
            snprintf(buffer, 32, "%02d:%02d:%02d", KOS::extRTC.getHour(e, e), KOS::extRTC.getMinute(), KOS::extRTC.getSecond());
        }
        canvas.setTextColor(TFT_WHITE);
        canvas.drawCenterString(String(buffer), centerX, y, &fonts::Orbitron_Light_24);

        bool wifi_connected = WiFi.status()==WL_CONNECTED;
        canvas.drawBitmap(centerX - 64 - 24, y+3, wifi_icon_allArray[wifi_connected], 24, 24, wifi_colors[wifi_connected]);

        int8_t batCharge = constrain(KOS::getBattery(), 0.0f, 1.0f) * 5;
        canvas.drawBitmap(centerX + 64, y+3, bat_icons[batCharge], 24, 24, bat_colors[batCharge]);

        return headerHeight;
    }

    void drawElements() {
        canvas.clear(TFT_BLACK);
        int lastY = -scrollY;

        lastY += drawHeader(lastY);

        // USBSerial.printf("Drawing %d elements\n", window.size());
        for(int i = 0; i < window.size(); i++) {
            // USBSerial.println(window[i].color);

            canvas.setTextDatum(textdatum_t::top_left);
            canvas.setFont(window[i].font);

            int numOfStrings = 1;
            for(int e = 0; e < window[i].text.length()-1; e++) {
                if(window[i].text[e] == '\n') numOfStrings++;
            }

            // int calculatedHeight = 12+canvas.fontHeight(&fonts::DejaVu18)*numOfStrings;

            
            int calculatedHeight;
            int realHeight;
            bool value;

            switch (window[i].type)
            {
            case ELEMENT_BUTTON:
                canvas.setTextColor(TFT_WHITE);
                calculatedHeight = 12+canvas.fontHeight(window[i].font)*numOfStrings;
                canvas.drawRoundRect(0, lastY, 239, calculatedHeight, 10, window[i].color);
                canvas.setCursor(5, lastY+8);
                canvas.print(window[i].text);
                if(i == activeElement) canvas.drawRoundRect(3, lastY+3, 239-6, calculatedHeight-6, 10-3, window[i].color);
                canvas.drawBitmap(240-6-20, lastY+6, button_arrow_img, 19, 16, window[i].color);
                lastY+=calculatedHeight+3;
                break;
            case ELEMENT_TEXT:
                if(i != activeElement) canvas.setTextColor(window[i].color);
                else canvas.setTextColor(TFT_ORANGE);
                canvas.setCursor(0, lastY+3);
                canvas.print(window[i].text);
                realHeight = canvas.getCursorY()-(lastY+3)+6+canvas.fontHeight(window[i].font);
                if(i == activeElement) canvas.drawFastVLine(0, lastY, realHeight, TFT_LIGHTGRAY);
                lastY+=realHeight;
                
                break;
            case ELEMENT_SWITCH:
                canvas.setTextColor(TFT_WHITE);
                calculatedHeight = 12+canvas.fontHeight(window[i].font)*numOfStrings;
                canvas.drawRoundRect(0, lastY, 239, calculatedHeight, 10, window[i].color);
                canvas.setCursor(5, lastY+8);
                canvas.print(window[i].text);
                if(i == activeElement) canvas.drawRoundRect(3, lastY+3, 239-6, calculatedHeight-6, 10-3, window[i].color);
                // canvas.drawBitmap(240-6-20, lastY+6, button_arrow_img, 19, 16, window[i].color);
                if(window[i].switchPointer != NULL) value = *window[i].switchPointer;
                else value  = window[i].virtualSwitch;

                if(value) {
                    canvas.drawRoundRect(240-6-30, lastY+6, 30, 17, 8, TFT_GREEN);
                    canvas.fillCircle(240-6-9, lastY+6+8, 6, TFT_SILVER);
                }
                else {
                    canvas.drawRoundRect(240-6-30, lastY+6, 30, 17, 8, TFT_RED);
                    canvas.fillCircle(240-6-30+9, lastY+6+8, 6, TFT_SILVER);
                }
                lastY+=calculatedHeight+3;
                break;
            
            
            default:
                break;
            }
        }
        allHeight = lastY + scrollY;
        if(allHeight > 240) {
            float scrollBarHeight = (240.0*240.0)/float(allHeight);
            canvas.drawFastVLine(239, (240.0-scrollBarHeight)*(scrollY/(allHeight-240.0)), scrollBarHeight, TFT_WHITE);
        }
        canvas.pushSprite(0, 0);
    }

    void screenUpdateHandler (void * arg) {
        while(true) {
            xSemaphoreTake(updateNeeded, 1000);
            // USBSerial.println("Updating screen now...");
            drawElements();

            vTaskDelay(5);

            if(scrollVelocity != 0) {
                
                if (scrollVelocity > 0 and scrollY <= allHeight-240) {
                    scrollY += scrollVelocity;
                    requestWindowUpdate();

                }
                if (scrollVelocity < 0 and scrollY >= 0) {
                    scrollY += scrollVelocity;
                    requestWindowUpdate();
                }
                
            }
            
        }
    }

    // void clickHandler (void * )

    TaskHandle_t taskHandleScreenUpdate;

    void initWindow() {
        if(!updateNeeded) updateNeeded = xSemaphoreCreateBinary();

        requestWindowUpdate();

        xTaskCreatePinnedToCore(
            screenUpdateHandler, 
            "KUI screen upd",
            10000, 
            NULL,
            3,
            &taskHandleScreenUpdate,
            0
        );

        KOS::onKeyPress(JOY_UP, [](uint8_t k) {
            if(KUI::activeElement > 0) KUI::activeElement --;
            requestWindowUpdate();
        });

        KOS::onKeyPress(JOY_DOWN, [](uint8_t k) {
            if(KUI::activeElement < KUI::window.size()-1) KUI::activeElement ++;
            requestWindowUpdate();
        });

        KOS::onKeyPress(JOY_CENTER, [](uint8_t k) {
            if(window[KUI::activeElement].type == ELEMENT_SWITCH){
                if(window[KUI::activeElement].switchPointer != NULL) *window[KUI::activeElement].switchPointer = !(*window[KUI::activeElement].switchPointer);
                window[KUI::activeElement].virtualSwitch = !window[KUI::activeElement].virtualSwitch;
            } 
            void (*fn)() = KUI::window[KUI::activeElement].onClick;
            if(fn != NULL) fn();
            
            requestWindowUpdate();
        });

        KOS::onKeyPress(BTN_DOWN, [](uint8_t k) {
            // USBSerial.println("SCROLL down");
            // USBSerial.println(KUI::scrollVelocityConst);
            KUI::scrollVelocity = KUI::scrollVelocityConst;
            requestWindowUpdate();
        });

        KOS::onKeyPress(BTN_UP, [](uint8_t k) {
            // USBSerial.println("SCROLL up");
            // USBSerial.println(-KUI::scrollVelocityConst);
            KUI::scrollVelocity = -KUI::scrollVelocityConst;
            requestWindowUpdate();
        });

        KOS::onKeyRelease(BTN_UP, [](uint8_t k) {KUI::scrollVelocity = 0;});
        KOS::onKeyRelease(BTN_DOWN, [](uint8_t k) {KUI::scrollVelocity = 0;});
    }

    void terminateWindow(bool resetPosition = true) {
        vTaskDelete(taskHandleScreenUpdate);
        KOS::detachAllKeys();
        if(resetPosition) {
            activeElement = 0;
            scrollY = 0;
        }
    }

};

