


namespace doodle_jump {
    #include "logo.h"






    uint32_t w;
    uint32_t h;

    uint16_t* doo_right;
    uint16_t* doo_left;
    uint16_t* tile_img;
    uint16_t* stick_tile_img;
    uint16_t* moving_tile_img;

    uint16_t** tiles_img[] = {&tile_img, &stick_tile_img, &moving_tile_img};

    float worldY = 0;

    int flySpeed = 4;

    float jumpSpeed = -4;

    float gravity = 0.06;

    struct {
    int vx = 0;
    float vy = -2;
    float x = 120;
    float y = 120;
    uint16_t *img;
    } doodle;


    struct Tile {
    float x;
    int y;

    float vx = 0;

    bool markRemoval = false;

    byte type = 0;

    Tile(int _x, int _y, byte _type = 0) {
        x = _x;
        y = _y;
        type = _type;

        vx = random(4, 24)/8.0;
    }
    };

    std::vector<Tile> tiles;

    void IRAM_ATTR goLeft(uint8_t k) {
    doodle.vx = -flySpeed;
    doodle.img = doo_left;
    }

    void IRAM_ATTR goRight(uint8_t k) {
    doodle.vx = flySpeed;
    doodle.img = doo_right;
    }

    void IRAM_ATTR noMove(uint8_t k) {
    doodle.vx = 0;
    }


    void main(void* p) {
        KOS::initSPIFFS();
        USBSerial.println("Doodle jump is starting...");

        doo_right = KOS::readImageBmp(SPIFFS, "/right.bmp", &w, &h);
        tile_img  = KOS::readImageBmp(SPIFFS, "/tile.bmp",  &w, &h);

        stick_tile_img  = KOS::readImageBmp(SPIFFS, "/stick-tile.bmp",  &w, &h);
        moving_tile_img  = KOS::readImageBmp(SPIFFS, "/moving-tile.bmp",  &w, &h);
        KOS::flipImageH(doo_right, &doo_left, 40, 30);

        doodle.img = doo_right;

        canvas.clear(TFT_WHITE);

        for(int i = 5; i > -3; i--) {
            int maxN = random(1, 3);
            for(int t = 0; t < maxN; t++) {
            tiles.push_back(Tile{random(0, 200), i*50+random(-15, 15)});
            }
        }


        KOS::onKeyPress(JOY_LEFT, goLeft);
        KOS::onKeyPress(JOY_RIGHT, goRight);
        KOS::onKeyRelease(JOY_LEFT, noMove);
        KOS::onKeyRelease(JOY_RIGHT, noMove);

        while(true) {
            canvas.clear(TFT_WHITE);
            for(int i = 0; i < tiles.size(); i++) {
                if(tiles[i].type == 2) {
                tiles[i].x+=tiles[i].vx;
                if(tiles[i].x < 10 or tiles[i].x > 190) tiles[i].vx*=-1;
                }
                canvas.pushImage(tiles[i].x, tiles[i].y+worldY, 40, 11, *tiles_img[tiles[i].type], TFT_WHITE);
                if(abs((doodle.x+20)-(tiles[i].x+20)) < 20 and abs(doodle.y-tiles[i].y+20) < 5 and doodle.vy > 0) {
                    doodle.vy = jumpSpeed;
                    KOS::playSound(&SPIFFS, "/jump.sound");
                    if(tiles[i].type == 1) {
                        tiles.erase(tiles.begin()+i);
                        i--;
                    }
                }
                if(tiles[i].y+worldY > canvas.height()+10) tiles[i].markRemoval = true;
            }

            for(int i = 0; i < tiles.size(); i ++) {
                if (tiles[i].markRemoval) {
                tiles.erase(tiles.begin()+i);
                i--;
                }
            }
            USBSerial.println(tiles.back().y+worldY);

            if(tiles.back().y+worldY > -80) {
                int maxN = random(1, 3);
                int lastY = tiles.back().y;
                for(int t = 0; t < maxN; t++) {
                tiles.push_back(Tile{random(0, 200), lastY-80+random(-30, 30), random(0, 3)});
                }
            }

            USBSerial.printf("Tiles length = %d\n", tiles.size());

            canvas.pushImage(round(doodle.x), round(doodle.y+worldY), 40, 30, doodle.img, TFT_WHITE);

            doodle.x+=doodle.vx;
            doodle.y+=doodle.vy;
            doodle.vy+=gravity;

            if((doodle.y+worldY) < 40 and doodle.vy < 0) worldY-=doodle.vy;

            if (doodle.y+worldY > canvas.height()-30) doodle.vy = jumpSpeed;

            canvas.pushSprite(0, 0);

            vTaskDelay(3);
        }

        vTaskDelete(NULL);


    }

    void init() {
        xTaskCreatePinnedToCore(
            main,
            "Doodle task",
            16384,
            NULL,
            1,
            NULL, 
            1
        );
    }
}