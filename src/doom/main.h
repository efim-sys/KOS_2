namespace doom {
    struct Block {
        bool present = true;
        lgfx::rgb565_t * texture;
        lgfx::rgb565_t color = TFT_LIGHTGRAY;

        Block(bool e, lgfx::rgb565_t* tex = NULL) {
            present = e;
            texture=tex;
        }
    };

    struct Point {
        float x;
        float y;
        Point(float _x = 9999, float _y = 9999) {
            x = _x; y = _y;
        }
    };

    struct Polygon {
        Point coords;
        Block* block;
        float dst;
        bool shade;
        int projectionX;
        Polygon(float x=9999, float y=9999, Block* b=NULL, bool shd=0) {
            coords.x = x;
            coords.y = y;
            block = b;
            shade = shd;
        }
    };

    int screenshot_id = 0;
    bool do_screenshot = false;

    lgfx::rgb565_t* globalTexture;
    uint16_t* uintTexture;

    std::vector <std::vector<Block>> area;
    std::vector <std::vector<bool>> area_temp = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1},
        {1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1},
        {1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1},
        {1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1},
        {1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1},
        {1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1},
        {1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    };

    struct Enemy {
        byte type = 0;
        uint16_t* textures[5];
        float x = 120;
        float y = 120;
        float speed = 1;
        int health = 100;
        bool drawn = false;

        // LGFX_Sprite sprite;

        std::vector<Point> waypoints;

        Enemy(float _x, float _y, uint16_t* tex) {
            x = _x; y = _y;
            textures[0] = tex;
        }
    };

    std::vector<Enemy> enemies;



    struct Player {
        float x = 120;
        float y = 120;
        float xspeed = 2.5;
        float yspeed = 2.5;
        float rspeed = PI/36.0;
        float r;
    } player;

    void generateArea() {
        for(int y = 0; y < area_temp.size(); y++) {
            area.push_back({});
            for(int x = 0; x < area_temp[y].size(); x++) {
                area[y].push_back(Block(area_temp[y][x], globalTexture));
            }
        }
    }

    float distance(Point a) {
        return hypot(fabsf(a.x), fabsf(a.y));
    }

    float blockSize = 20.0f;

    bool renderMethod = false;
    byte wallType = 0;
    bool texturesOn = true;

    lgfx::rgb565_t* Twall;
    lgfx::rgb565_t* T1488;
    lgfx::rgb565_t* Tbird;

    Polygon traceRay(float dr) {

        float ry, rx;

        int crossX, crossY;

        uint maxDepth = 10;

        uint dof;

        float rotation = player.r+dr;

        // Horizontal Walls

        if(rotation > TWO_PI) rotation -= TWO_PI;
        if(rotation < 0) rotation += TWO_PI;

        if(rotation < PI) ry = blockSize-fmodf(player.y, blockSize);
        else              ry = -fmodf(player.y, blockSize);

        

        for(dof = 1; dof < maxDepth; dof++) {
            
            rx = ry*(1.0f/tanf(rotation));

            rx = max(rx, -3000.0f);
            rx = min(rx, 3000.0f);

            crossX = floorf((player.x+rx) / blockSize);

            if(ry>0) crossY = ceilf((player.y+ry) / blockSize);
            else crossY = floorf((player.y+ry) / blockSize)-0.5f;

            crossY = constrain(crossY, 0, area.size()-1);
            crossX = constrain(crossX, 0, area[crossY].size()-1);
            

            if(area[crossY][crossX].present) {
                break;
            }
            
            if(ry>0) ry+=blockSize;
            else ry-=blockSize;      

        }
        
        Polygon Hcross;
        if(dof<maxDepth) Hcross = {rx, ry, &area[crossY][crossX], 0};

        // With vertical walls

        if(rotation < M_PI_2 or rotation > 3*M_PI_2) rx = blockSize-fmodf(player.x, blockSize);
        else                                         rx = -fmodf(player.x, blockSize);

        for(dof = 1; dof < maxDepth; dof++) {

            
            ry = rx*(tanf(rotation));

            ry = constrain(ry, -3000.0f, 3000.0f);

            crossY = floorf((player.y+ry) / blockSize);

            if(rx>=0) crossX = ceilf((player.x+rx) / blockSize);
            else crossX = floorf((player.x+rx) / blockSize)-0.5f;

            crossY = constrain(crossY, 0, area.size()-1);
            crossX = constrain(crossX, 0, area[crossY].size()-1);

            // USBSerial.printf("crossy = %d, crossx = %d\n", crossY, crossX);
            
            if(area[crossY][crossX].present) {
                break;
            }
            
            if(rx>0) rx+=blockSize;
            else rx-=blockSize;      

        }
        Polygon Vcross;
        if(dof<maxDepth) Vcross = {rx, ry, &area[crossY][crossX], 1};

        Polygon cross;

        switch (wallType) {
            case 0:
                cross = (distance(Hcross.coords) < distance(Vcross.coords)) ? Hcross : Vcross;
                break;
            case 1:
                cross = Vcross;
                break;
            case 2:
                cross = Hcross;
                break;
        }

        return cross;
    }

    void render2D() {
        canvas.clear(TFT_BLACK);
        // Draw Grid
        for(uint y = 0; y < area.size(); y ++) {
            for(uint x = 0; x < area[y].size(); x++) {
                canvas.drawRect(x*20, y*20, 20, 20, TFT_DARKGRAY);
                if(area[y][x].present) canvas.fillRect(x*20+1, y*20+1, 18, 18, area[y][x].color);
            }
        }

        // draw player

        canvas.fillCircle(player.x, player.y, 3, TFT_YELLOW);

        int numOfRays = 240;
        float FOV = PI/2;

        for(int ray = -numOfRays/2; ray < numOfRays/2; ray ++) {
            float dr = ray * (FOV/(numOfRays));
            Polygon cross = traceRay(dr);
            float dst = distance(cross.coords);
            int height = 120-constrain(dst, 0.0f, 120.0f);
            canvas.drawLine(player.x, player.y, player.x+cross.coords.x, player.y+cross.coords.y, TFT_YELLOW);
        }

        for(Enemy e : enemies) {
            canvas.fillCircle(e.x, e.y, 3, TFT_GREEN);
        }
        

        USBSerial.printf("X=%f, Y=%f, R=%f\n", player.x, player.y, player.r);

        canvas.pushSprite(0, 0);
    }

    

    

    void render3D() {

        int numOfRays = display.width();
        float FOV = PI/2;

        canvas.fillRect(0, 0, display.width(), display.height()/2, TFT_BLUE);
        canvas.fillRect(0, display.height()/2, display.width(), display.height()/2, TFT_DARKGRAY);

        

        // USBSerial.printf("Enemy Angle = %f", enemyAngle);

        Polygon scan[display.width()];

        for(int ray = -numOfRays/2; ray < numOfRays/2; ray ++) {
            float dr = ray * (FOV/(numOfRays));
            // Polygon cross = traceRay(dr);
            scan[numOfRays/2+ray]  = traceRay(dr);
            scan[numOfRays/2+ray].dst = distance(scan[numOfRays/2+ray].coords)*cosf(dr);
            scan[numOfRays/2+ray].projectionX = ray+(numOfRays>>1);
        }

        auto comp = [](Polygon a, Polygon b) {
            return a.dst > b.dst;
        };

        std::sort(scan, scan+numOfRays, comp);

        // bool enemyDrawn = false;

        float enemyXConst = float(display.width()/2)/sinf(FOV/2.0f);

        for(int i = 0; i < enemies.size(); i++) {
            enemies[i].drawn = false;
        }

        for(int ray = 0; ray < numOfRays; ray ++) {
            Polygon* cross = &scan[ray];
            int height = (20.0*120.0)/constrain(cross->dst, 1.0f, float(display.width()));
            height = max(height, 0);

            for(int i = 0; i < enemies.size(); i++) {
                Enemy* guard = &enemies[i];

                float enemyDst = hypotf(player.x-guard->x, player.y-guard->y);
                float enemyAngle = atan2f(player.x-guard->x, player.y-guard->y)-PI;

                enemyAngle = enemyAngle+player.r - M_PI_2;

                if(enemyAngle > PI) enemyAngle  -= 2.0f*PI;
                if(enemyAngle < -PI) enemyAngle += 2.0f*PI;

                
                if(!guard->drawn and (ray == (numOfRays-1) or enemyDst > cross->dst)) {
                    // USBSerial.printf("EnemyDst = %f; wallDst = %f\n", enemyDst, cross->dst);
                    
                    // USBSerial.printf("enemyAngle = %f\n", enemyAngle);
                    if(enemyAngle < FOV and enemyAngle > - FOV) {
                        int enemyX = (numOfRays/2)-enemyXConst*sinf(enemyAngle);
                        float dimension = (40.0/enemyDst);
                        canvas.pushImageRotateZoom(enemyX, display.height()/2, 32, 32, 0, dimension, dimension, 64, 64, guard->textures[0], 0xE007);
                    }
                    // else USBSerial.printf("Enemy out of FOV; EnemyAngle = %f\n", enemyAngle);
                    guard->drawn = true;
                }
            }

            



            int startX = cross->projectionX;
            int startY = (display.height()/2)-(height>>1);
            if(!texturesOn) canvas.drawFastVLine(startX, startY, height, (cross->shade) ? TFT_GREEN : TFT_DARKGREEN);
            else{
                int textureX;

                if(!cross->shade) textureX = fmodf(player.x+cross->coords.x, 20.0f) * (128.0f/20.0f);
                else textureX = fmodf(player.y+cross->coords.y, 20.0f) * (128.0f/20.0f);
                for(int pixelY = 0; pixelY<height; pixelY++) {
                    if(startY+pixelY < 0 or startY+pixelY > display.height()) continue;
                    int textureY = map(pixelY, 0, height, 0, 127);
                    
                    lgfx::rgb565_t color = cross->block->texture[textureY*128+textureX];
                    if(cross->shade) {
                        color = lgfx::rgb565_t(color.R8()>>1, color.G8()>>1, color.B8()>>1);
                    }

                    
                    canvas.drawPixel(startX, startY+pixelY, color);
                }
            }
        }

        

        // USBSerial.printf("X=%f, Y=%f, R=%f\n", player.x, player.y, player.r);

        canvas.pushSprite(0, 0);

        }
        

        // for(int ray = -numOfRays/2; ray < numOfRays/2; ray ++) {
        //     float dr = ray * (FOV/(numOfRays));
        //     Polygon cross = traceRay(dr);
        //     float dst = distance(cross.coords)*cosf(dr);

        //     int height = (20.0*120.0)/constrain(dst, 1.0f, 240.0f);
        //     // height = constrain(height, 0, 240);
        //     height = max(height, 0);

        //     int startX = ray+(numOfRays>>1);
        //     int startY = 120-(height>>1);
        //     if(!texturesOn) canvas.drawFastVLine(startX, startY, height, (cross.shade) ? TFT_GREEN : TFT_DARKGREEN);
        //     else{
        //         int textureX;
        //         float rayR = player.r+dr;
        //         if(!cross.shade) textureX = fmodf(player.x+cross.coords.x, 20.0f) * (128.0f/20.0f);
        //         else textureX = fmodf(player.y+cross.coords.y, 20.0f) * (128.0f/20.0f);
        //         // if(ray == 0)USBSerial.println(textureX);
        //         for(int pixelY = 0; pixelY<height; pixelY++) {
        //             int textureY = map(pixelY, 0, height, 0, 127);
        //             // if(ray == 0) {
        //             //     USBSerial.println(textureY);

        //             //     USBSerial.printf("TEXTUREX = %d, textureY = %d\n", textureX, textureY);
        //             //     USBS              //     USBSerial.println(textureY*128+textureX);
        //             //     USBSerial.println(128*128);
        //             // }
        //             lgfx::rgb565_t color = cross.block->texture[textureY*128+textureX];
        //             if(cross.shade) {
        //                 color = lgfx::rgb565_t(color.R8()>>1, color.G8()>>1, color.B8()>>1);
        //             }

                    
        //             canvas.drawPixel(startX, startY+pixelY, color);
        //         }
        //     }
        // }
        
    

    bool keypresses[8] = {0,0,0,0,0,0,0,0};

    void IRAM_ATTR kpressHandler(uint8_t k) {
        keypresses[k]=true;
    }

    void IRAM_ATTR kreleaseHandler(uint8_t k) {
        keypresses[k]=false;
    }

    void movingHandler(void * arg) {
        while(true) {          
            for(int i = 0; i < 8; i ++) {
                keypresses[i] = digitalRead(buttons[i].pin);
            }
            float yoff = (player.yspeed+blockSize/2)*sinf(player.r);
            float xoff = (player.xspeed+blockSize/2)*cosf(player.r);
            if(keypresses[JOY_UP]) {
                if(!area[(player.y+yoff) / 20.0][player.x / 20.0].present) player.y += (player.yspeed)*sinf(player.r);
                if(!area[player.y / 20.0][(player.x+xoff) / 20.0].present) player.x += (player.xspeed)*cosf(player.r);
            }
            if(keypresses[JOY_DOWN]) {
                if(!area[(player.y-yoff) / 20.0][player.x / 20.0].present) player.y -= (player.yspeed)*sinf(player.r);
                if(!area[player.y / 20.0][(player.x-xoff) / 20.0].present) player.x -= (player.xspeed)*cosf(player.r);
            }
            // if(keypresses[JOY_UP]) {
            //     player.y -= player.yspeed;
            // }
            // if(keypresses[JOY_DOWN]) {
            //     player.y += player.yspeed;
            // }
            // if(keypresses[JOY_LEFT]) {
            //     player.x -= player.xspeed;
            // }
            // if(keypresses[JOY_RIGHT]) {
            //     player.x += player.xspeed;
            // }
            if(keypresses[JOY_LEFT]) player.r -= player.rspeed;
            if(keypresses[JOY_RIGHT]) player.r += player.rspeed;

            if(player.r > TWO_PI) player.r -= TWO_PI;
            if(player.r < 0) player.r += TWO_PI;
            vTaskDelay(50);
        }
    }

    void applyNtohs(lgfx::rgb565_t* p) {
        for(int i = 0; i < 128*128; i++) {
            p[i] = ntohs((uint16_t) p[i]);
        }

        
    }

    void main(void * arg) {
        KOS::initSPIFFS();
        uint32_t texw, texh;
        // T1488 =  (lgfx::rgb565_t*) KOS::readImageBmp(SPIFFS, "/1488.bmp", &texw, &texh);
        Twall =  (lgfx::rgb565_t*) KOS::readImageBmp(SPIFFS, "/wall.bmp", &texw, &texh);
        Tbird =  (lgfx::rgb565_t*) KOS::readImageBmp(SPIFFS, "/bird.bmp", &texw, &texh);

        enemies = {
            Enemy(120, 120, KOS::readImageBmp(SPIFFS, "/guard-walk1.bmp")), 
            Enemy(40, 40, KOS::readImageBmp(SPIFFS, "/guard-shoot.bmp")), 
            Enemy(120, 140, KOS::readImageBmp(SPIFFS, "/guard-walk2.bmp"))
        };
        // enemies[0].textures[0] = KOS::readImageBmp(SPIFFS, "/guard-walk1.bmp");

        // applyNtohs(T1488);
        applyNtohs(Twall);
        applyNtohs(Tbird);        

        area = {
            {{1, Twall}, {1, Tbird}, {1, Twall}, {1, Twall}, {1, Twall}, {1, Twall}, {1, Twall}, {1, Twall}, {1, Twall}, {1, Twall}, {1, Twall}, {1, Twall}, {1, Twall}, {1, Twall}},
            {{1, Twall}, {0}, {0},  {1,Twall}, {0}, {0}, {0}, {0}, {0}, {1, Twall}, {0}, {0}, {0}, {1, Twall}},
            {{1, Twall}, {0}, {0},  {1,Twall}, {0}, {0}, {0}, {0}, {0}, {1, Twall}, {0}, {0}, {0}, {1, Twall}},
            {{1, Twall}, {0}, {0},  {1,Twall}, {0}, {0}, {1, Twall}, {0}, {0}, {1, Twall}, {0}, {0}, {0}, {1, Twall}},
            {{1, Twall}, {0}, {0},  {1,Twall}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {1, Twall}},
            {{1, Twall}, {0}, {0}, {1, Twall}, {0}, {0}, {0}, {0}, {1, Twall}, {1, Twall}, {1, Twall}, {1, Twall}, {1, Twall}, {1, Twall}},
            {{1, Twall}, {0}, {0}, {1, Twall}, {0}, {0}, {0}, {0}, {1, Twall}},
            {{1, Twall}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {1, Twall}},
            {{1, Twall}, {1, Tbird}, {1, Twall}, {1, Twall}, {1, Twall}, {1, Twall}, {1, Twall}, {1, Twall}, {1, Twall}}
        };


        player.x = 240;
        player.y = 30;
        player.r = 2;

        KOS::onKeyPress(BTN_BOOT, [](uint8_t k){
            do_screenshot = true;
        });


        // for(uint i = 0; i < 8; i ++) {
        //     KOS::onKeyPress(i, kpressHandler);
        //     KOS::onKeyRelease(i, kreleaseHandler);
        // }

        // KOS::onKeyPress(BTN_BOOT, [](byte k){
        //     renderMethod = !renderMethod;
        // });

        // KOS::onKeyPress(BTN_UP, [](byte k){
        //     wallType = (wallType+1)%3;
        // });

        // KOS::onKeyPress(BTN_DOWN, [](byte k){
        //     texturesOn = !texturesOn;
        // });

        // generateArea();

        xTaskCreate(movingHandler, "mvg hd", 2048, NULL, 5, NULL);

        KOS::initSD();

        while(true) {
            if(renderMethod)render2D();
            else render3D();

            if(do_screenshot) {
                do_screenshot = false;

                char screenshot_filename[64] = "";
                sprintf(screenshot_filename, "/sdcard/doom_screenshot%03d.bmp", screenshot_id++);
                KOS::saveFramebuffer((uint16_t*) canvas.getBuffer(), canvas.width(), canvas.height(), screenshot_filename);
            }

            vTaskDelay(10);
        }
    }

    void init() {
        xTaskCreatePinnedToCore(main, "doom task", 16384, NULL, 5, NULL, 1);
    }
}