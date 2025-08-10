#include <ArduinoWebsockets.h>


namespace robot_control {
    const char* server_host = "192.168.4.1/ws";
    const uint16_t server_port = 80;

    websockets::WebsocketsClient client;

    struct Quad {
        double x, y, z, w;
    };

    Quad position = {200, 0, 0, 90};

    bool magnetEnabled = false;

    struct {
        float a = 200.0;
        float b = 245.0;
        float c = 40.0;
    } len;

    Quad calcIK(Quad in) {
        float r = hypotf(in.x, in.y);
        float angle_t = atan2f(in.y, in.x);

        float x = r;
        float y = in.z;

        float w = radians(in.w);

        float x2 = x-cosf(w)*len.c;
        float y2 = y+sinf(w)*len.c;

        float distance = hypotf(x2, y2);

        if(distance > len.a+len.b) {
            USBSerial.println("Point out of bounds!");
        }

        Quad out;

        out.x = degrees(angle_t);
        out.y = degrees(PI/2 - (acosf((len.a*len.a + distance*distance - len.b*len.b)/(2*len.a*distance)) + ((y2>=0)?1:-1)*acosf(x2 / distance)));
        out.z = degrees(PI - acos((len.a*len.a + len.b*len.b - distance*distance)/(2.0*len.a*len.b)));
        out.w = degrees(PI/2 - radians(out.y) - radians(out.z) + w);

        return out;
    }

    bool sendQuad(Quad data) {
        char buffer[128];

        snprintf(buffer, 128, 
            "{\"mag\":%d,\"pos\":[%f,%f,%f,%f]}",
            magnetEnabled,
            data.x,
            data.y,
            data.z,
            data.w
        );

        USBSerial.println(buffer);
        
        return client.send(buffer);
    }

    struct Sixt {
        float x, y, z, w, w1, w2 = 0;
    };

    bool sendSixt(Sixt data) {
        char buffer[128];

        snprintf(buffer, 128, 
            "{\"mag\":%d,\"pos\":[%f,%f,%f,%f,%f,%f]}",
            magnetEnabled,
            data.x,
            data.y,
            data.z,
            data.w,
            data.w1,
            data.w2
        );

        USBSerial.println(buffer);
        
        return client.send(buffer);
    }

    bool sendData() {
        auto data = calcIK(position);

        return sendQuad(data);
    }

    SemaphoreHandle_t sm = xSemaphoreCreateBinary();

    int mode = -1;

    void directControl() {
        bool t = sendData();

        // KUI::window.push_back(KUI::Element(ELEMENT_BUTTON, t?"Success":"Fault", NULL, TFT_WHITE, NULL));
        KUI::terminateWindow();

        KOS::initSPIFFS();

        uint16_t* img = KOS::readImageBmp(SPIFFS, "/6dof.bmp");

        display.clear(TFT_WHITE);

        display.pushImage(0, 0, 240, 240, img);

        KOS::detachAllKeys();



        xSemaphoreTake(sm, 1);

        KOS::onKeyPress(JOY_UP, [](uint8_t k) {
            position.x += 10;
            xSemaphoreGive(sm);
        });
        KOS::onKeyPress(JOY_DOWN, [](uint8_t k) {
            position.x -= 10;
            xSemaphoreGive(sm);
        });
        KOS::onKeyPress(JOY_RIGHT, [](uint8_t k) {
            position.y += 10;
            xSemaphoreGive(sm);
        });
        KOS::onKeyPress(JOY_LEFT, [](uint8_t k) {
            position.y -= 10;
            xSemaphoreGive(sm);
        });

        KOS::onKeyPress(BTN_UP, [](uint8_t k) {
            position.z += 10;
            xSemaphoreGive(sm);
        });
        KOS::onKeyPress(BTN_DOWN, [](uint8_t k) {
            position.z -= 10;
            xSemaphoreGive(sm);
        });

        KOS::onKeyPress(JOY_CENTER, [](uint8_t k) {
            magnetEnabled = !magnetEnabled;
            xSemaphoreGive(sm);
        });

        while(true) {
            xSemaphoreTake(sm, portMAX_DELAY);


            t = sendData();

            if(!t) {
                client.connect("ws://192.168.4.1/ws");
                xSemaphoreGive(sm);
            }

            USBSerial.println(t);

            delay(200);
        }

        vTaskDelete(NULL);
    }

    std::vector<std::vector<Sixt>> preset = {
        {{20, 0, 0, 0}, {-20, 0, 0, 0}, {0, 0, 0, 0}},
        {{0, 20, 0, 0}, {0, -20, 0, 0}, {0, 0, 0, 0}},
        {{0, 0, 20, 0}, {0, 0, -20, 0}, {0, 0, 0, 0}},
        {{0, 0, 0, 20}, {0, 0, 0, -20}, {0, 0, 0, 0}},
        {{0, 0, 0, 0, 60, 0}, {0, 0, 0, 0, -60, 0}, {0, 0, 0, 0, 0, 0}},
        {{0, 0, 0, 0, 0, 60}, {0, 0, 0, 0, 0, -60}, {0, 0, 0, 0, 0, 0}},
    };

    std::vector<Sixt> tasks;

    bool skip_connection = false;

    bool gotReady = false;



    void waitForReady(uint32_t timeout = 5000) {
        uint32_t tmr = millis() + timeout;
        while(millis() < tmr) {
            client.poll();
            if(gotReady) {
                gotReady = false;
                break;
            }
            vTaskDelay(100);
        }
    }

    void main(void * param){


        KUI::window = {
            KUI::Element(ELEMENT_TEXT, "Connecting to 4dof-robot...", NULL, TFT_ORANGE, NULL),
            KUI::Element(ELEMENT_TEXT, "Elapsed: 0 ms", NULL, TFT_GREEN, NULL) ,
            KUI::Element(ELEMENT_BUTTON, "Skip Connection", NULL, TFT_WHITE, [](){
                skip_connection = true;
                USBSerial.printf("skip_connection = %d\n", skip_connection);
            })       };
        KUI::initWindow();

        WiFi.begin("6dof-robot", "mcpe7891");
        uint64_t connectionStart = millis();
        while((WiFi.status() != WL_CONNECTED) and !skip_connection) {
            KUI::window[1].text = "Elapsed: " +   String((millis()-connectionStart)) + "ms";
            KUI::requestWindowUpdate();
            delay(300);
        }

        client.connect("ws://192.168.4.1/ws");

        KUI::window = {
            KUI::Element(ELEMENT_BUTTON, "Direct control", NULL, TFT_ORANGE, [](){mode = 0; xSemaphoreGive(sm);}),
            KUI::Element(ELEMENT_BUTTON, "Axis 1 sweep", NULL, TFT_ORANGE, [](){mode = 1; xSemaphoreGive(sm);}),
            KUI::Element(ELEMENT_BUTTON, "Axis 2 sweep", NULL, TFT_ORANGE, [](){mode = 2; xSemaphoreGive(sm);}),
            KUI::Element(ELEMENT_BUTTON, "Axis 3 sweep", NULL, TFT_ORANGE, [](){mode = 3; xSemaphoreGive(sm);}),
            KUI::Element(ELEMENT_BUTTON, "Axis 4 sweep", NULL, TFT_ORANGE, [](){mode = 4; xSemaphoreGive(sm);}),
            KUI::Element(ELEMENT_BUTTON, "Axis 5 sweep", NULL, TFT_ORANGE, [](){mode = 5; xSemaphoreGive(sm);}),
            KUI::Element(ELEMENT_BUTTON, "Axis 6 sweep", NULL, TFT_ORANGE, [](){mode = 6; xSemaphoreGive(sm);}),
            KUI::Element(ELEMENT_BUTTON, "Rectangle", NULL, TFT_ORANGE, [](){mode = 7; xSemaphoreGive(sm);}),
        };

        KUI::requestWindowUpdate();

        xSemaphoreTake(sm, 1);

        client.onMessage([](auto msg){
            USBSerial.println(msg.data());
            gotReady = true;
        });
        
        while(true) {
            xSemaphoreTake(sm, portMAX_DELAY);
            if(mode == 0) directControl();

            tasks = preset[mode-1];

            sendSixt(tasks[0]);
            waitForReady();
            sendSixt(tasks[1]);
            waitForReady();
            sendSixt(tasks[2]);
            waitForReady();
            

        }
    }

    

    void init() {
        xTaskCreatePinnedToCore(main, "robo-c", 16384, NULL, 5, NULL, 1);
    }


}