namespace clockApp {
    uint32_t tmr;

    struct tm timeinfo;

    bool firstDraw = true;

    void drawClock() {
    
    canvas.setColor(TFT_CYAN);
    int halfScreen = display.width() >> 1;
    int clockRad =  halfScreen - 40;

    canvas.fillCircle(halfScreen, halfScreen, clockRad-25+1, TFT_BLACK);

    if(firstDraw) {
        canvas.drawCircle(halfScreen, halfScreen, clockRad, TFT_CYAN);
        canvas.drawCircle(halfScreen, halfScreen, clockRad+1, TFT_CYAN);
    }
    if(firstDraw) {
        for(int i = 0; i < 12; i++) {
        float angle = (i / 12.0) * PI * 2.0 + HALF_PI;
        int x1 = halfScreen - cos(angle) * clockRad;
        int y1 = halfScreen - sin(angle) * clockRad;
        int x2 = halfScreen - cos(angle) * (clockRad - 20);
        int y2 = halfScreen - sin(angle) * (clockRad - 20);
        // canvas.setColor(TFT_CYAN);
        canvas.drawLine(x1, y1, x2, y2, TFT_CYAN);

        if(firstDraw) {
            canvas.setFont(&fonts::Orbitron_Light_24);
            x1 = halfScreen - cos(angle) * (clockRad+25) - (canvas.textWidth(String(i)) >> 1);
            y1 = halfScreen - sin(angle) * (clockRad+25) - (canvas.fontHeight()>>1);

            canvas.setCursor(x1, y1);

            // USBSerial.printf("num: %d len: %d\n", i, canvas.textWidth(String(i)));
            canvas.print(String(i));
        }
        }

        for(int i = 0; i < 60; i++) {
        float angle = (i / 60.0) * PI * 2.0 + HALF_PI;
        int x1 = halfScreen - cos(angle) * (clockRad - 1);
        int y1 = halfScreen - sin(angle) * (clockRad - 1);
        int x2 = halfScreen - cos(angle) * (clockRad - 10);
        int y2 = halfScreen - sin(angle) * (clockRad - 10);

        canvas.drawLine(x1, y1, x2, y2, TFT_CYAN);
        }
    }
    getLocalTime(&timeinfo);
    
    int hour = timeinfo.tm_hour;
    int minute = timeinfo.tm_min;
    int ms = timeinfo.tm_sec*1000;

    // canvas.setColor(TFT_RED);
    float angle = (ms / 60000.0) * PI * 2.0 + HALF_PI;
    int x1 = halfScreen - cos(angle) * (clockRad-25);
    int y1 = halfScreen - sin(angle) * (clockRad-25);
    canvas.drawLine(x1, y1, halfScreen, halfScreen, TFT_RED);

    canvas.setColor(TFT_GREEN);
    angle = (minute / 60.0) * PI * 2.0 + HALF_PI;
    x1 = halfScreen - cos(angle) * (clockRad-30);
    y1 = halfScreen - sin(angle) * (clockRad-30);
    canvas.drawLine(halfScreen, halfScreen, x1, y1);
    // canvas.drawWideLine(x1, y1, halfScreen, halfScreen, 2, TFT_GREEN);

    // canvas.setColor(TFT_MAGENTA);
    angle = (hour / 12.0) * PI * 2.0 + HALF_PI;
    x1 = halfScreen - cos(angle) * (clockRad-50);
    y1 = halfScreen - sin(angle) * (clockRad-50);
    canvas.drawLine(x1, y1, halfScreen, halfScreen, TFT_MAGENTA);

    // canvas.setCursor(0, 0);
    // canvas.setFont(&fonts::AsciiFont8x16);
    // canvas.printf("T=%.1f", tsens);
    canvas.setCursor(0, 240);
    canvas.setFont(&fonts::AsciiFont8x16);
    canvas.setTextDatum(bottom_right);
    canvas.printf("%.1f FPS", 1000000.0/(micros()-tmr));
    tmr = micros();
    canvas.setTextDatum(top_left);
    canvas.pushSprite(0, 0);
    firstDraw = false;
    }

    void main(void* arg) {
        KOS::initWiFi();

        
        KOS::initRTC();
        display.clear(TFT_BLACK);
        canvas.clear(TFT_BLACK);
        
        canvas.setTextColor(TFT_GREEN, TFT_BLACK);

        canvas.setFont(&fonts::Orbitron_Light_24);

        canvas.setTextSize(1);

        while(true) {
            drawClock();
            vTaskDelay(500);
        }
    }

    void init() {
        xTaskCreatePinnedToCore(main, "clock", 4096, NULL, 3, NULL, 0);
    }
}