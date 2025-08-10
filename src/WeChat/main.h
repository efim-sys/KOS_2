#include <esp_now.h>
#include <map>

namespace WeChat {
    uint8_t sendTo[] = {0xF0, 0xf5, 0xbd, 0x43, 0x60, 0xec};

    char myName[32] = "Anonimus";

    struct Message {
        uint64_t timestamp;
        char message[100];
        char nickname[32] = "Anonimus";
    };

    // struct Contact {
    //     String name = "Anonimus";
    //     uint8_t mac[6];
    //     Contact(String _name, uint64_t _mac) {
    //         memcpy(mac, &_mac, 6);
    //         name = _name;
    //     }
    // };

    // Contact contacts[] = {
    //     {"Green",  0xf0f5bd4360ec},
    //     {"Gray",   0xf0f5bd4360ec},
    //     {"Golden", 0xf0f5bd4360ec},
    // };

    std::map<uint64_t, String> names;

    std::vector <Message> messageHistory;

    void onDataReceive(const uint8_t* mac, const uint8_t* data, int len) {
        digitalWrite(LED_BUILTIN, HIGH);
        if(len != sizeof(Message)) {
            KUI::window.push_back(
                KUI::Element(ELEMENT_TEXT, "ERR;LEN "+String(len), NULL, TFT_RED, NULL)
            );
        }
        else {
            Message m = *((Message *) data);
            KUI::window.push_back(
                KUI::Element(ELEMENT_TEXT, String(m.nickname) + ": "+ String(m.message), NULL, TFT_GREEN, NULL)
            );
            
            messageHistory.push_back(m);
            KUI::requestWindowUpdate();
        }
        vTaskDelay(100);
        digitalWrite(LED_BUILTIN, LOW);
    }

    uint8_t contacts[][6] {
        {0xf0, 0xf5, 0xbd, 0x43, 0x60, 0xec},
        {0xd8, 0x3b, 0xda, 0xa4, 0x6b, 0xfc},
        {0xf0, 0xf5, 0xbd, 0x43, 0x55, 0x20}
    };

    esp_now_peer_info_t peerInfo;

    SemaphoreHandle_t sm = xSemaphoreCreateBinary();

    uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    Message msProto;

    void readName() {
        KOS::initSPIFFS();

        File conf = SPIFFS.open("/WeChat.conf");
        
        if(conf) {
            strcpy(myName, conf.readString().c_str());
        }

        conf.close();
    }

    void main(void * arg) {
        pinMode(LED_BUILTIN, OUTPUT);
        names[0xEC6043BDF5F0] = "mr Green";
        names[0xFC6BA4DA3BD8] = "mr Gray";
        names[0x205543BDF5F0] = "mr Golden";

        readName();

        // if(names.count(ESP.getEfuseMac())) strcpy(myName, names[ESP.getEfuseMac()].c_str());

        

        USBSerial.println(ESP.getEfuseMac(), HEX);

        KUI::scrollY = 0;
        KUI::activeElement = 0;
        KUI::window = {
            KUI::Element(ELEMENT_TEXT, "My nickname:\n" + String(myName), NULL, TFT_WHITE, NULL ), 
            KUI::Element(ELEMENT_BUTTON, "Send message", NULL, TFT_SKYBLUE, [] {
                xSemaphoreGive(sm);
            } ), 
        };

        KUI::initWindow();

        WiFi.mode(WIFI_STA);
        if (esp_now_init() != ESP_OK) {
            USBSerial.println("Error initializing ESP-NOW");
            return;
        }

        peerInfo.channel = 0;  
        peerInfo.encrypt = false;

        for(uint i = 0; i < sizeof(contacts); i+=6) {
            memcpy(peerInfo.peer_addr, contacts+i, 6);
            if (esp_now_add_peer(&peerInfo) != ESP_OK){
                USBSerial.println("Failed to add peer");
                return;
            }
        }
        memcpy(peerInfo.peer_addr, broadcastAddress, 6);
        esp_now_add_peer(&peerInfo);

        esp_now_register_recv_cb(esp_now_recv_cb_t(onDataReceive));

        KOS::autoSleep::enable = false;
        
        xSemaphoreTake(sm, 1);

        while(true) {
            xSemaphoreTake(sm, portMAX_DELAY);
            KUI::terminateWindow();
            String msg = KOS::keyboard("Type message");
            strcpy(msProto.message, msg.c_str());
            strcpy(msProto.nickname, myName);
            esp_now_send(broadcastAddress, (uint8_t *) &msProto, sizeof(Message));

            KUI::window.push_back(
                KUI::Element(ELEMENT_TEXT, String(msProto.nickname) + ": "+ String(msProto.message), NULL, TFT_GREEN, NULL)
            );
            
            messageHistory.push_back(msProto);

            KUI::requestWindowUpdate();
            KUI::initWindow();
        }
    }

    void changeName(void * arg) {
        KUI::terminateWindow();

        readName();

        String newName = KOS::keyboard("Your new name:", String(myName));

        // KOS::initSPIFFS();

        File conf = SPIFFS.open("/WeChat.conf", "w", true);
        conf.print(newName);
        conf.close();
        KUI::initWindow();

        vTaskDelete(NULL);
    }

    void init() {
        xTaskCreatePinnedToCore(
            main,
            "WeChat process",
            8192,
            NULL,
            5,
            NULL,
            1
        );
    }
}