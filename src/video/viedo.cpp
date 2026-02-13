#include "video.h"
#include "KUI/KUI.h"


fs::FS filesys = SD_MMC;
String filename = "/Video/CostaRica.VidBMP";

uint32_t framesCount;

int32_t currentFrame = 0;

SemaphoreHandle_t sm = xSemaphoreCreateBinary();

bool pause_flag = false;
bool performExit = false;

bool hasSound = false;
bool soundLoop = true;

TaskHandle_t videoPlTask;

void video_main(void * arg) {
    // KOS::initSD();
    uint16_t* frame = (uint16_t*) ps_malloc(240*240*3);
    uint32_t w, h = 0;
    
    uint32_t filesPerFolder;
    String fileNameTemplate;

    USBSerial.println(filename);
    USBSerial.println("SUSUSUSUSUSUSUSUSUSU");

    File description = filesys.open(filename);

    String descPath = String(description.path());

    descPath = descPath.substring(0, descPath.lastIndexOf('/'));

    USBSerial.printf("descpription path = %s\n", descPath); 

    fileNameTemplate = description.readStringUntil('\n');
    fileNameTemplate.trim();
    USBSerial.printf("FilenameTemplate = %s\n", fileNameTemplate.c_str()); 

    framesCount = description.readStringUntil('\n').toInt();
    USBSerial.printf("FramesCount = %d\n", framesCount); 

    filesPerFolder = description.readStringUntil('\n').toInt();
    USBSerial.printf("FilesPerFolder = %d\n", filesPerFolder);

    fileNameTemplate = descPath+fileNameTemplate;
    USBSerial.printf("FilenameTemplate = %s\n", fileNameTemplate.c_str()); 

    
    String soundFile = description.readStringUntil('\n');
    soundFile.trim();
    
    hasSound = (soundFile[0] == '/');
    soundFile = descPath + soundFile;

    USBSerial.printf("Has sound = %d\n", hasSound);
    USBSerial.printf("Sound = %s\n", soundFile.c_str());

    description.close();

    performExit = false;

    xSemaphoreGive(sm);

    char filename[100];

    KOS::onKeyPress(JOY_RIGHT, [](uint8_t k){
        currentFrame+=100;
        if(currentFrame>framesCount) currentFrame = framesCount; 
    });

    KOS::onKeyPress(JOY_LEFT, [](uint8_t k){
        currentFrame-=100;
        if(currentFrame>framesCount) currentFrame = 0; 
    });

    KOS::onKeyPress(JOY_CENTER, [](uint8_t k){
        pause_flag=!pause_flag;
        if(pause_flag and hasSound and KOS::soundPlayTask != NULL) {
            vTaskSuspend(KOS::soundPlayTask);
            ledcWriteTone(1, 0);
        }
        if(!pause_flag) {
            xSemaphoreGive(sm);
            if(hasSound and KOS::soundPlayTask != NULL) vTaskResume(KOS::soundPlayTask);
        }
    });


    KOS::onKeyPress(BTN_BOOT, [](uint8_t k){
        
        performExit = true;
        xSemaphoreGive(sm);
    });

    uint64_t tmr = micros();

    if(hasSound and KOS::soundPlayTask != NULL) {
        vTaskDelete(KOS::soundPlayTask);
        KOS::soundPlayTask = NULL;
        ledcWriteTone(1, 0);
    }

    while(true) {
        
        for(currentFrame = 1; currentFrame < framesCount; currentFrame++) {
            xSemaphoreTake(sm, portMAX_DELAY);

            if(KOS::soundPlayTask == NULL and hasSound) KOS::playSound(&SD_MMC, soundFile);

            if(performExit) {
                if(hasSound and KOS::soundPlayTask != NULL) {
                    vTaskDelete(KOS::soundPlayTask);
                    KOS::soundPlayTask = NULL;
                    ledcWriteTone(1, 0);
                }
                KUI::initWindow();
                
                vTaskDelete(NULL);

            }
            snprintf(filename, 100, fileNameTemplate.c_str(), (currentFrame-1)/filesPerFolder , currentFrame);
            tmr=micros();
            KOS::readImageBmp(SD_MMC, filename, &w, &h, frame);
            // USBSerial.printf("ReadImage time = %d ", micros()-tmr);
            
            display.pushImage(0, 0, w, h, frame);
            display.fillRect(0, 238, (currentFrame*240)/framesCount, 2, TFT_RED);
            if(!pause_flag) xSemaphoreGive(sm);
            else {
                display.fillRect(120-30, 120-30, 25, 60, TFT_LIGHTGRAY);
                display.fillRect(120+5, 120-30, 25, 60, TFT_LIGHTGRAY);
            }
            // USBSerial.printf("FPS=%f\n", 1000000.0/float(micros()-tmr));
            // tmr = micros();
            
            vTaskDelay(1);
        }
    }
}

// fs::FS _fs = SD_MMC, String _filename = "/Video/CostaRica.VidBMP"

void video_init(fs::FS _fs, String _filename) {
    filesys = _fs;
    filename = _filename;

    

    xTaskCreate(video_main, "Video pl", 8192, videoPlTask, 10, NULL);
}
