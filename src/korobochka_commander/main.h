#include <functional>

namespace korobochka_commander {
    std::vector<String> files;

    String currentFile = "/";

    SemaphoreHandle_t dirSwitch = xSemaphoreCreateBinary();

    int ae;
    int sy;
    std::vector<KUI::Element> snapshot;

    void showImage(void * arg) {
        uint32_t w, h = 0;
        String path = currentFile+"/"+files[KUI::activeElement-1];
        // vTaskDelay(100);
        USBSerial.println(path);
        uint16_t* img = KOS::readImageBmp(SD_MMC, path.c_str(), &w, &h);

        display.pushImage(0, 0, w, h, img);

        free(img);
        vTaskDelete(NULL);
    }

    

    void showText(void * arg) {
        String path = currentFile+"/"+files[KUI::activeElement-1];
        ae = KUI::activeElement;
        sy = KUI::scrollY;
        snapshot = KUI::window;

        KUI::activeElement = 0;
        KUI::scrollY = 0;

        USBSerial.println(path);
        

        File f = SD_MMC.open(path);
        String content = f.readString();

        KUI::window = {
            KUI::Element(ELEMENT_BUTTON, "exit", NULL, TFT_RED, [](){
                KUI::activeElement = ae;
                KUI::scrollY = sy;
                KUI::window = snapshot;
            }),
            KUI::Element(ELEMENT_TEXT, content, NULL, TFT_WHITE, NULL, &fonts::DejaVu9)
        };

        KUI::requestWindowUpdate();
        vTaskDelete(NULL);
    }

    



    void generateMenu(fs::FS &fs, String dirname) {
        KUI::window={KUI::Element{ELEMENT_BUTTON, ".. back", NULL, TFT_RED, [](){
            if (currentFile == "/") return;
            currentFile = currentFile.substring(0, currentFile.lastIndexOf("/"));
            xSemaphoreGiveFromISR(dirSwitch, NULL);
        }}};

        KUI::activeElement = 0;
        KUI::scrollY = 0;
        
        File root = fs.open(dirname.c_str());
        if (!root) {
            USBSerial.println("Failed to open directory");
            return;
        }
        if (!root.isDirectory()) {
            USBSerial.println("Not a directory");
            return;
        }

        File file = root.openNextFile();

        files.clear();

        
        
        while(file) {
            if(file.isDirectory()) {
                KUI::window.push_back(KUI::Element{ELEMENT_BUTTON, file.name(), NULL, TFT_CYAN, []() {
                    currentFile+=("/"+files[KUI::activeElement-1]);
                    xSemaphoreGiveFromISR(dirSwitch, NULL);
                }});
            }
            else if(String(file.name()).endsWith(".bmp")) KUI::window.push_back(KUI::Element{ELEMENT_TEXT, file.name(), NULL, TFT_RED, []{
                xTaskCreatePinnedToCore(showImage, "showImage", 8192, NULL, 4, NULL, 1);
            }});
            else if(String(file.name()).endsWith(".VidBMP")) KUI::window.push_back(KUI::Element{ELEMENT_TEXT, file.name(), NULL, TFT_PURPLE, []{
                if(digitalRead(0)) {
                    video::init(SD_MMC, String(currentFile+"/"+files[KUI::activeElement-1]));
                    KUI::terminateWindow(false);
                }
                else xTaskCreatePinnedToCore(showText, "showText", 8192, NULL, 4, NULL, 0);
                
            }});
            else if(String(file.name()).endsWith(".obj")) {
                KUI::window.push_back(KUI::Element{ELEMENT_TEXT, file.name(), NULL, TFT_RED, []{
                    if(digitalRead(0)) {
                        viewer3D::init(String(currentFile+"/"+files[KUI::activeElement-1]));
                    }
                    else xTaskCreatePinnedToCore(showText, "showText", 8192, NULL, 4, NULL, 0);
            }});  
            }
            else if(String(file.name()).endsWith(".sound")) KUI::window.push_back(KUI::Element{ELEMENT_TEXT, file.name(), NULL, TFT_BLUE, []{
                
                if(digitalRead(0)) {
                    KOS::playSound(&SD_MMC, String(currentFile+"/"+files[KUI::activeElement-1]));
                }
                else xTaskCreatePinnedToCore(showText, "showText", 8192, NULL, 4, NULL, 0);
            }});
            else KUI::window.push_back(KUI::Element{file.isDirectory() ? ELEMENT_BUTTON : ELEMENT_TEXT, file.name(), NULL, TFT_GREEN, []{
                xTaskCreatePinnedToCore(showText, "showText", 8192, NULL, 4, NULL, 0);
            }});
            files.push_back(String(file.name()));
            file = root.openNextFile();
        }

        KUI::requestWindowUpdate();
    }

    void main(void* p) {
        KOS::initSPIFFS();
        KOS::initSD();
        
        KUI::initWindow();
        xSemaphoreGive(dirSwitch);
        while(true) {
            xSemaphoreTake(dirSwitch, portMAX_DELAY);
            generateMenu(SD_MMC, currentFile);
        }
        
        vTaskDelete(NULL);
    }

    void init() {
        xTaskCreatePinnedToCore(
            main,
            "KC task",
            16384,
            NULL,
            1,
            NULL, 
            1
        );
    }
}