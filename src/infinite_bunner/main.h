


namespace infinite_bunner {
    #include "logo.h"
    #include "textures.h"

    volatile int32_t x = 130;
    volatile int32_t y = 230;

    volatile int32_t myrow = 0;
    volatile int32_t mycol = 7;

    volatile int32_t vx = 20;
    volatile int32_t vy = 20;

    uint16_t* rabbit_img;

    uint16_t* image_lib[bmp_allArray_LEN];

    uint16_t* rabbit_lib[rabbit_allArray_LEN];

    uint16_t* train_lib[train_allArray_LEN];

    uint16_t* tree_lib[tree_allArray_LEN];

    int32_t score = 0;

    struct Row {
    uint16_t *picture;
    uint8_t type;
    int32_t y;

    int8_t trees[12] = {0,0,0,0,0,0,0,0,0,0,0,0};

    Row(int32_t yi, uint8_t ti, uint16_t *pi) {
        y = yi;
        type = ti;
        picture = pi;
        if(type == 66 or type == 0) return;
        for(int i = 0; i < 4; i ++) {
            trees[esp_random()%12] = 1+(esp_random()%2);
        }
    }
    };

    void messageBox(String txt) {
        
        display.fillRect(15, 15, 240-30, 240-30, TFT_LIGHTGREY);
        display.fillGradientRect(15, 15, 240-30, 15, TFT_BLUE, TFT_MAGENTA, lgfx::v1::gradient_fill_styles::HLINEAR);
        display.drawRect(14, 14, 240-28, 17, TFT_WHITE);
        display.drawRect(14, 14, 240-28, 240-30, TFT_WHITE);
        display.setTextColor(TFT_BLACK);
        display.setFont(&fonts::AsciiFont8x16);
        display.setCursor(15+5, 35);
        display.print(txt);

    }

    void gameOver() {
        KOS::detachAllKeys();

        KOS::playSound(&SPIFFS, "/die.sound");
        // display.clear(TFT_RED);
        // display.setTextColor(TFT_WHITE);
        // display.setCursor(0, 0);
        

        // display.fillGradientRect(0, 0, 240, 240, TFT_BLUE, TFT_MAGENTA, lgfx::v1::gradient_fill_styles::HLINEAR);

        // display.print("Game Over");

        String s = "Game Over!\n   Final score was: ";
        s+=score;
        messageBox(s);
        
        delay(100000);
        ESP.restart();
    }

    struct Train {
        uint16_t* picture;
        int direction = 1;
        int32_t x;
        int32_t y;

        uint32_t nextArrive;

        void reset() {
            nextArrive = millis() + esp_random()%3000;
            if (direction > 0) {
                picture = train_lib[1];
                x = -430;
            }
            else {
                picture = train_lib[0];
                x = 240;
            }
        }

        Train(int32_t yi) {
            
            direction = (esp_random()%2) ? 10:-10;
            y = yi;
            reset();
        }

        void update() {
            if(millis() < nextArrive) return;
            x+=direction;
            canvas.pushImage(x, y-30, 430, 67, (lgfx:: rgb565_t*) picture, TFT_BLACK);
            if(infinite_bunner::x > x and infinite_bunner::x < x+430 and (infinite_bunner::y-10) == y) gameOver();
            // USBSerial.printf("ZY=%d, TY=%d\n", ::y, y);
            if (x > 240 or x < -430) reset();
        }
    };

    std::vector<Row> area;

    std::vector<Train> trains;

    void IRAM_ATTR upFn(uint8_t key) {
        if(y>0 and area[myrow+1+2].trees[mycol] == 0) {
			y-=vy;
			score++;
			myrow++;
		}
		rabbit_img = rabbit_lib[0];
    }

    void IRAM_ATTR dnFn(uint8_t key) {
        if(myrow > 0 and area[myrow-1+2].trees[mycol] == 0) {
			y+=vy;
			score--;
			myrow--;
		}
		rabbit_img = rabbit_lib[2];
    }

    void IRAM_ATTR rFn(uint8_t key) {
        if (x < 200 and area[myrow+2].trees[mycol+1] == 0) {
			x+=vx;
			mycol++;
		}
		rabbit_img = rabbit_lib[1];
    }

    void IRAM_ATTR lFn(uint8_t key) {
        if(x>0 and area[myrow+2].trees[mycol-1] == 0) {
			x-=vx;
			mycol--;
		}
		rabbit_img = rabbit_lib[3];
    }

    int32_t getLastY() {
        if (area.size() == 0) return canvas.height();
        return area.back().y;
    }

    void fillArea() {
        while(area.size()<16) {
        uint32_t rd = esp_random();
        // USBSerial.println(rd);
        if(rd%3 == 0) {
            area.push_back(Row(getLastY()-20, 91, image_lib[1]));
            area.push_back(Row(getLastY()-20, 66, image_lib[2]));
            trains.push_back(Train(getLastY()));
            area.push_back(Row(getLastY()-20, 91, image_lib[3]));
        }
        else area.push_back(Row(getLastY()-20, 91, image_lib[0]));
    }
    }

    static LGFX_Sprite spriteHighscore(&canvas);
    static LGFX_Sprite spriteScore(&canvas);
    





    void main(void* p) {
        USBSerial.println("Bunner is starting...");


        for(int i = 0; i < rabbit_allArray_LEN; i ++) {
            rabbit_lib[i] = (uint16_t*) ps_malloc(2*20*20);
            memcpy(rabbit_lib[i], rabbit_allArray[i], 2*20*20);
        }

        rabbit_img = rabbit_lib[0];
        //   delay(1000);

        for(int i = 0; i < bmp_allArray_LEN; i ++) {
            image_lib[i] = (uint16_t*) ps_malloc(2*240*20);
            memcpy(image_lib[i], bmp_allArray[i], 2*240*20);
        }

        for(int i = 0; i < train_allArray_LEN; i ++) {
            train_lib[i] = (uint16_t*) ps_malloc(2*	430*67);
            memcpy(train_lib[i], train_allArray[i], 2*430*67);
        }

        for(int i = 0; i < tree_allArray_LEN; i ++) {
            tree_lib[i] = (uint16_t*) ps_malloc(2*20*20);
            memcpy(tree_lib[i], tree_allArray[i], 2*20*20);
        }

        USBSerial.println("filling vector");
        // delay(1000);
        area.push_back(Row(getLastY()-20, 0, image_lib[0]));
        area.push_back(Row(getLastY()-20, 0, image_lib[0]));
        area.push_back(Row(getLastY()-20, 0, image_lib[0]));
        
        fillArea();

        for(int i = 0; i < area.size(); i++) {
            USBSerial.printf("ID: %d, y: %d, type: %d, pic: %d\n", i, area[i].y, area[i].type, area[i].picture);
        }

        pinMode(18, OUTPUT);



        // canvas.setPsram(true);

        // canvas.createSprite(240,240);

        canvas.setFont(&fonts::Orbitron_Light_32);

        spriteScore.setPsram(true);
        spriteHighscore.setPsram(true);

        spriteHighscore.createSprite(120, 32);
        spriteScore.createSprite(120, 32);

        

        spriteHighscore.setFont(&fonts::Orbitron_Light_32);
        spriteScore.setFont(&fonts::Orbitron_Light_32);


        KOS::onKeyPress(JOY_UP, upFn);
        KOS::onKeyPress(JOY_DOWN, dnFn);
        KOS::onKeyPress(JOY_LEFT, lFn);
        KOS::onKeyPress(JOY_RIGHT, rFn);

        uint64_t fpsTimer = 0;

        uint32_t currentTick = 0;   

        while(true) {
            for(int i = 0; i < area.size(); i ++) {
                canvas.pushImage(0, area[i].y, 240, 20, (lgfx:: rgb565_t*) area[i].picture);
                for(int8_t tree=0; tree < 12; tree++) {
                    if(!area[i].trees[tree]) continue;
                    canvas.pushImage(tree*20, area[i].y, 20, 20, (lgfx:: rgb565_t*) tree_lib[area[i].trees[tree]-1], TFT_BLACK);
                }
            }
            //   canvas.fillCircle(x, y, 5, TFT_ORANGE);
            // canvas.pushImage(0, 0, 240, 20, (lgfx:: rgb565_t*) grass_img);
            // g1.show(120);
            canvas.pushImage(x+10, y-10, 20, 20, (lgfx:: rgb565_t*) rabbit_img, TFT_BLACK);

            for(int i = 0; i < trains.size(); i++) {
                trains[i].update();
            }
                spriteHighscore.clear(TFT_BLACK);
                spriteHighscore.setTextColor(TFT_GOLD, TFT_BLACK);
                spriteHighscore.setCursor(0, 0);
                float fps = 1000000.0/(micros()-fpsTimer);
                fpsTimer = micros();
                spriteHighscore.print(fps);
                USBSerial.printf("FPS=%f", fps);
                
                spriteHighscore.pushSprite(0, 0, TFT_BLACK);

                spriteScore.clear(TFT_BLACK);
                spriteScore.setTextColor(TFT_SILVER, TFT_BLACK);
                spriteScore.setCursor(0, 0);
                spriteScore.print(score);
                spriteScore.pushSprite(0, 32, TFT_BLACK);

            canvas.pushSprite(0, 0);

                uint32_t delta;
                if (y >120) delta = 1;
                else delta = (120/y);
                // USBSerial.printf("Delta = %d", delta);
                if(delta >= 2 or currentTick%2) {
                    y+=delta;
                    for(int i = 0; i < area.size(); i ++) {
                        area[i].y+=delta;
                    }
                    for(int i = 0; i < trains.size(); i++) {
                        trains[i].y+=delta;
                    }
                    if(area.front().y > canvas.height()) {
                        area.erase(area.begin());
                        myrow--;
                    }

                    if(trains.front().y > canvas.height()+40) trains.erase(trains.begin());
                    fillArea();

                    if (y > canvas.height()+20) gameOver();
                    // USBSerial.printf("Vector size is %d\n", area.size());
                }	
                currentTick++;
        }

        vTaskDelete(NULL);
    }

    void init() {
        xTaskCreatePinnedToCore(
            main,
            "Bunner task",
            16384,
            NULL,
            1,
            NULL, 
            1
        );
    }
}