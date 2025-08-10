namespace stopWatch {
    uint64_t elapsed = 0;
    uint64_t prev    = 0;

    SemaphoreHandle_t sm = xSemaphoreCreateBinary();

    bool running = false;

    void IRAM_ATTR startTimer() {
        prev = micros();
        running = true;
        xSemaphoreGive(sm);
    }

    void IRAM_ATTR stopTimer() {
        running = false;
    }

    void IRAM_ATTR resetTimer() {
        elapsed = 0;
        stopTimer();
    }

    

    void main(void * arg) {
        
        KUI::window={
            KUI::Element(ELEMENT_TEXT, "12::00::99", NULL, TFT_GREENYELLOW, NULL, &fonts::DejaVu56),
            KUI::Element(ELEMENT_BUTTON, "Start", NULL, TFT_GREENYELLOW, startTimer),
            KUI::Element(ELEMENT_BUTTON, "Stop",  NULL, TFT_ORANGE, stopTimer),
        };

        KUI::initWindow();

        xSemaphoreGive(sm);

        prev = 0;

        while(true) {
            xSemaphoreTake(sm, portMAX_DELAY);

            uint64_t mcs = micros();
            elapsed += (mcs-prev);
            prev = mcs;

            KUI::window[0].text = String(elapsed/1000000.0);
            KUI::requestWindowUpdate();

            vTaskDelay(100);

            if(running) xSemaphoreGive(sm);
        }
    }

    void init() {
        
        
        

        // KUI::terminateWindow();
        xTaskCreatePinnedToCore(
            main,
            "stopWatch",
            16384,
            NULL,
            1,
            NULL, 
            1
        );
    }
}