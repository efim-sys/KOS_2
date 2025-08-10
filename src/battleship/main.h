namespace BattleShip
{
    uint16_t* shipTextures[16];
    uint16_t* waterTextures[4];


    const int tileSize = 24;

    uint32_t worldTick = 0;

    void textureBatch(fs::FS &fs, uint16_t** to, const char* name_temp, uint32_t firstID, uint32_t lastID) {
        for(int i = firstID; i <= lastID; i++) {
            char filename[64];
            snprintf(filename, 64, name_temp, i);
            to[i] = KOS::readImageBmp(fs, filename);

            
        }
    }

    int32_t worldx, worldy;

    struct Ship {
        double x, y;
        double angle;

        double hspeed, vspeed;

        double maxSpeed = 10;
        double accel = 1;

        float health;
    } player;

    void drawOcean() {
        int32_t startx = worldx;
        int32_t starty = worldx;

        for(int i = 0; i < 100; i++) {
            canvas.pushImage(startx + (i%10)*tileSize, starty + (i/10)*tileSize, tileSize, tileSize, waterTextures[(worldTick>>4)%4]);
        }
    }

    void main(void * arg) {
        KOS::initSPIFFS();
        
        textureBatch(SPIFFS, shipTextures, "/pt-%04d.bmp", 0, 15);
        textureBatch(SPIFFS, waterTextures, "/w2-%04d.bmp", 0, 3);

        KOS::onKeyPress(JOY_UP, [](uint8_t k) {
            if(hypot(player.hspeed, player.vspeed) < player.maxSpeed) {
                player.hspeed += 1;
            }
        }); 

        for(;;) {
            drawOcean();

            worldTick++;
            
            canvas.pushImage(20, 20, 56, 56, shipTextures[worldTick%16], ntohs(TFT_BLUE));

            canvas.pushSprite(0, 0);

            delay(100);
        }
    }

    void init() {
        xTaskCreatePinnedToCore(
            main,
            "BS task",
            32768,
            NULL,
            1,
            NULL, 
            1
        );
    }
} 
