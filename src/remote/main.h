namespace TVremote {
    std::vector<uint32_t> signal;

    SemaphoreHandle_t startScan = xSemaphoreCreateBinary();

    uint32_t lastInt;

    void signalHandle() {
        signal.push_back(micros() - lastInt);
        digitalWrite(1, !digitalRead(18));
        lastInt = micros();
    }

    void sendSignal(void* p) {
        pinMode(10, OUTPUT);
        USBSerial.println(ledcAttachChannel(10, 38000, 8, 3));
        // ledcWriteChannel(3, 127);
        bool st = false;
        for(uint32_t i : signal) {
            delayMicroseconds(i);
            // USBSerial.println(i);
            ledcWriteChannel(3, 127*(!st));
            st=!st;
        }
        delay(50);
        ledcDetach(10);
        pinMode(10, INPUT);
        
    }

    void sendSignalFile(void * p) {
        String fname = KUI::window[KUI::activeElement].text;
        File file = SPIFFS.open("/"+fname);

        signal.clear();

        // if(!file) {
        //     USBSerial.println("ERR open file "+fname);
        // }
        // else USBSerial.println("OK open file "+fname);

        // USBSerial.printf("file.available()=%d\n", file.available());
        // USBSerial.printf("file=%d\n", file);
        // USBSerial.printf("file.size=%d\n", file.size());
        while(file.available()) {
            uint32_t tmp;

            
            file.readBytes((char*) &tmp, 4);
            // USBSerial.println(tmp);
            signal.push_back(tmp);
        }

        file.close();

        // for(uint32_t i : signal) {
        //     USBSerial.print(i);
        //     USBSerial.print(' ');
        // }
        // USBSerial.println();

        sendSignal(NULL);
        vTaskDelete(NULL);
    }

    void genMenu(void);
    
    void deleteSignal(void *p) {
        SPIFFS.remove("/"+KUI::window[KUI::activeElement].text);
        KUI::terminateWindow();
        genMenu();

        vTaskDelete(NULL);
    }

    void copySignal(void * p) {
        
        String s = "/"+KUI::window[KUI::activeElement].text;
        KOS::copyFile(SPIFFS, SD_MMC, s.c_str(), s.c_str());

        genMenu();
        vTaskDelete(NULL);
    }

    std::vector<String> signals;


    void genMenu() {
        
        KUI::activeElement = 0;
        KUI::window = {
            KUI::Element(ELEMENT_BUTTON, "Scan",  NULL, TFT_GREEN, [](){
                xSemaphoreGive(startScan);
            }),
        };

        File root = SPIFFS.open("/");

        File file = root.openNextFile();

        signals.clear();

        while(file) {
            String fname = file.name();
            USBSerial.println(fname);
            file.close();
            if(fname.endsWith(".IR")) {
                KUI::window.push_back(
                    KUI::Element(ELEMENT_BUTTON, fname,  NULL, TFT_SKYBLUE, [](){
                        xTaskCreatePinnedToCore(sendSignalFile, "sendTV", 8192, NULL, 8, NULL, 0);
                    })
                );

                signals.push_back(fname);
            }
            file = root.openNextFile();
        }

        KUI::window.push_back(
            KUI::Element(ELEMENT_BUTTON, "Delete file...", NULL, TFT_RED, [](){
                KUI::window = {
                    KUI::Element(ELEMENT_TEXT, "Select file to remove:",  NULL, TFT_WHITE, NULL)
                };

                for(String i : signals) {
                    KUI::window.push_back(
                        KUI::Element(ELEMENT_BUTTON, i,  NULL, TFT_RED, [](){
                            xTaskCreatePinnedToCore(deleteSignal, "sigDel", 4096, NULL, 8, NULL, 0);

                        })
                    );
                }

                KUI::requestWindowUpdate();
            })
        );

        KUI::window.push_back(
            KUI::Element(ELEMENT_BUTTON, "Copy to SD...", NULL, TFT_YELLOW, [](){
                KUI::window = {
                    KUI::Element(ELEMENT_TEXT, "Select file to copy:",  NULL, TFT_WHITE, NULL)
                };

                for(String i : signals) {
                    KUI::window.push_back(
                        KUI::Element(ELEMENT_BUTTON, i,  NULL, TFT_YELLOW, [](){
                            xTaskCreatePinnedToCore(copySignal, "sigCopy", 4096, NULL, 8, NULL, 0);
                        })
                    );
                }   
                KUI::activeElement = 1;
                KUI::requestWindowUpdate();

            })
        );

        KUI::initWindow();
    }

    void main(void * p) {
        KOS::initSD();
        genMenu();

        xSemaphoreTake(startScan, 1);

        pinMode(1, OUTPUT);
        // xTaskCreatePinnedToCore(recvIR, "recvIR", 16384, NULL, 10, NULL, 1);

        for(;;) {
            xSemaphoreTake(startScan, portMAX_DELAY);
            pinMode(1, OUTPUT);
            signal.clear();
            
            attachInterrupt(18, signalHandle, CHANGE);

            delay(5000);

            detachInterrupt(18);
            pinMode(1, INPUT);

            signal[0] = 10;

            String fname = "/"+KOS::keyboard("Signal name:")+".IR";

            File file = SPIFFS.open(fname, FILE_WRITE, true);

            if(!file) {
                USBSerial.println("ERR open file "+fname);
            }
            else  USBSerial.println("OK open file "+fname);

            uint8_t* addr = (uint8_t*) signal.data();
            USBSerial.println(file.write(addr, signal.size()*sizeof(uint32_t)));

            // for(uint32_t i = 0; i < signal.size()*sizeof(uint32_t); i++) {
            //     USBSerial.println(addr[i]);
            // }

            file.close();

            genMenu();

            
        }
                
    }

    void init() {
        xTaskCreate(main, "remoteTV", 8192, NULL, 1, NULL);
    }
}