namespace launcher {
    #include "export.h"
    
    exp_os os;

    void dsp(void) {
        display.pushImage(0, 0, 240, 280, os.fb);
    }

    void main(void * p) {
        
        os.fb = (uint16_t*) calloc(240*280, sizeof(uint16_t));
        os.display = dsp;

        os.putc = [](char c) {
            USBSerial.print(c);
        };

        os.putd = [](int d) {
            USBSerial.println(d);
        };

        void* program = heap_caps_malloc(1024, MALLOC_CAP_EXEC | MALLOC_CAP_8BIT);
        USBSerial.printf("PROGRAM=%p\n", program);

        KOS::initSD();

        const char* filename = "/linked.bin";

        File file = SD_MMC.open(filename);

        if(!file) USBSerial.printf("Error openeing file %s\n", filename);
        else USBSerial.printf("Opened file %s\n", filename);

        size_t len = 0;
        while(file.available()) {
            uint32_t tmp;

            file.readBytes((char*) &tmp, 4);
            
            ((uint32_t*)program)[len/4] = tmp;

            len+=4;
        }

        // char* e = (char*) malloc(1024);

        // len = file.readBytes(e, 1024);

        // size_t len = file.readBytes((char*) &program, 1024);

        USBSerial.printf("len = %u\n", len);

        file.close();

        USBSerial.printf("READING PROGRAM DATA:\n");
        for(size_t i = 0; i < len/4; i++) {
            USBSerial.printf("[%x]", ((uint32_t*)program)[i]);
        }
        USBSerial.printf("\n\nEND READING\n\n");

        int (*func)(exp_os*) = (int (*)(exp_os*)) program;
        USBSerial.printf("FUNC ptr=%d\n", (int) &os);
        USBSerial.printf("Returns %d\n", func(&os));

        // display.clear(TFT_GREEN);

        while(1) vTaskDelay(1000);

    }

    void init() {
        xTaskCreate(main, "BIN launcher", 8192, NULL, 1, NULL);
    }
}