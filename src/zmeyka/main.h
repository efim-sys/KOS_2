namespace zmeyka{
    #include "textures.h"

    bool appleEaten = false;
    
    int fieldh, fieldw = 12;

    uint16_t* heads[4];
    uint16_t* bodies[2];
    uint16_t* turns[4];
    uint16_t* tails[4];

    uint16_t* appleImg;

    uint16_t** imageSet[4][4] = {
    {&bodies[0], &turns[0], &bodies[0], &turns[3]},
    {&turns[0], &bodies[0], &turns[1], &bodies[1]},
    {&bodies[0], &turns[1], &bodies[0], &turns[2]},
    {&turns[3], &bodies[1], &turns[2], &bodies[0]},
    };

    void randomBlinkTask(void* arg) {
            pinMode(LED_BUILTIN, OUTPUT);
        while(true) {
            digitalWrite(LED_BUILTIN, 1);
            vTaskDelay(100);
            digitalWrite(LED_BUILTIN, 0);
            vTaskDelay(esp_random()%1000+300);
        }
    }

    int getWorldDirection(int x1, int y1, int x2, int y2) {
        if(x1 > x2) return 2;
        if(x1 < x2) return 0;
        if(y1 > y2) return 3;
        return 1;
    }

    void drawGrid() {
    for(int x = 0; x < fieldw; x++) {
        for(int y = 0; y < fieldh; y++) {
            canvas.fillRect(20*x, 20*y, 20, 20, (((x+y)%2) == 0) ? display.color24to16(0x5a8000):TFT_GREENYELLOW);
        }
    }
    }

    struct snakeElement {
    int x;
    int y;
    uint16_t color = TFT_RED;

    snakeElement(int _x, int _y, uint16_t _color = TFT_RED) {
        x = _x;
        y = _y;
        color = _color;
        }
    };

    std::vector<snakeElement> snake;

    int vx = -1;
    int vy = 0;

    void moveSnake() {
    int lastX = snake.back().x;
    int lastY = snake.back().y;
    for(int i = snake.size()-1; i > 0; i--) {
        snake[i].x = snake[i-1].x;
        snake[i].y = snake[i-1].y;
    }
    if(appleEaten) {
        snake.push_back(snakeElement{lastX, lastY});
        appleEaten = false;
    }
    snake[0].x+=vx;
    snake[0].y+=vy;
    }

    void drawSnake() {
    for(int i = 1; i < snake.size()-1; i++) {
        // canvas.fillRect(snake[i].x*20+1, snake[i].y*20+1, 18, 18, snake[i].color);
        int direction1 = getWorldDirection(snake[i].x, snake[i].y, snake[i-1].x, snake[i-1].y);
        int direction2 = getWorldDirection(snake[i].x, snake[i].y, snake[i+1].x, snake[i+1].y);
        uint16_t* img = *imageSet[direction1][direction2];
        USBSerial.printf("D1=%d, D2=%d\n addr=%d rjrj=%d\n", direction1, direction2, img, turns[1]);
        
        canvas.pushImage(snake[i].x*20, snake[i].y*20, 20, 20, (lgfx::rgb565_t*) img, TFT_BLACK);
    }
    canvas.pushImage(snake[0].x*20, snake[0].y*20, 20, 20, (lgfx::rgb565_t*) heads[getWorldDirection(snake[1].x, snake[1].y, snake[0].x, snake[0].y)], TFT_BLACK);
    int sz = snake.size()-1;
    canvas.pushImage(snake[sz].x*20, snake[sz].y*20, 20, 20, (lgfx::rgb565_t*) tails[getWorldDirection(snake[sz].x, snake[sz].y, snake[sz-1].x, snake[sz-1].y)], TFT_BLACK);
    }

    void sendInfo() {
    for(int i = 0; i < snake.size(); i++) {
        // canvas.fillRect(snake[i].x*20+1, snake[i].y*20+1, 18, 18, snake[i].color);
        USBSerial.printf("ID=%d; x=%d; y = %d\n", i, snake[i].x, snake[i].y);
    }
    USBSerial.println("\n");
    }

    uint16_t* rotateImageRight(uint16_t* from, int w, int h) {
    uint16_t* addr = (uint16_t*) ps_malloc(w*h*2);
    for(uint32_t i = 0; i < w*h; i++) {
        addr[i] = from[w*h - w - w*(i%h) + i/w];
    }
    return addr;
    }

    void goUp(uint8_t key) {
    if(vy == 1) return;
    vx = 0;
    vy = -1;
    }

    void goLeft(uint8_t key) {
    if(vx == 1) return;
    vy = 0;
    vx = -1;
    }
    void goRight(uint8_t key) {
    if(vx == -1) return;
    vy = 0;
    vx = 1;
    }
    void goDown(uint8_t key) {
    if(vy == 1) return;
    vx = 0;
    vy = 1;
    }

    struct {
    int x = 0;
    int y = 0;
    } apple;



    uint16_t* copyImageToPsram(uint16_t* from, int w, int h) {
    uint16_t* addr = (uint16_t*) ps_malloc(w*h*2);
    memcpy(addr, from, w*h*2);
    return addr;
    }

    void gameOver() {
        display.setTextSize(4);
        display.setTextDatum(textdatum_t::middle_center);
        display.setTextColor(TFT_RED);
        display.drawString("Game Over!", 120, 120);
        KOS::playSound(&SPIFFS, "/die.sound");
        delay(100000);
        ESP.restart();
    }

    void main(void * p) {
        // xTaskCreatePinnedToCore(randomBlinkTask, "blinker", 1024, NULL, 0, NULL, 0);
        #ifdef IPS169
        fieldh = 14;
        #endif

        fieldw = 12;
        
        heads[0] = copyImageToPsram((uint16_t*) zmeyka_pmem_head, 20, 20);
        heads[1] = rotateImageRight(heads[0], 20, 20);
        heads[2] = rotateImageRight(heads[1], 20, 20);
        heads[3] = rotateImageRight(heads[2], 20, 20);

        tails[0] = copyImageToPsram((uint16_t*) zmeyka_pmem_tail, 20, 20);
        tails[1] = rotateImageRight(tails[0], 20, 20);
        tails[2] = rotateImageRight(tails[1], 20, 20);
        tails[3] = rotateImageRight(tails[2], 20, 20);

        turns[0] = copyImageToPsram((uint16_t*) zmeyka_pmem_turn, 20, 20);
        turns[1] = rotateImageRight(turns[0], 20, 20);
        turns[2] = rotateImageRight(turns[1], 20, 20);
        turns[3] = rotateImageRight(turns[2], 20, 20);

        bodies[0] = copyImageToPsram((uint16_t*) zmeyka_pmem_body, 20, 20);
        bodies[1] = rotateImageRight(bodies[0], 20, 20);

        appleImg = copyImageToPsram((uint16_t *) zmeyka_pmem_apple, 20, 20);

        snake.push_back(snakeElement{5, 5});
        snake.push_back(snakeElement{5, 6});
        snake.push_back(snakeElement{5, 7});
        snake.push_back(snakeElement{5, 8});
        snake.push_back(snakeElement{5, 9});

        drawGrid();

        KOS::onKeyPress(JOY_UP, goUp);
        KOS::onKeyPress(JOY_DOWN, goDown);
        KOS::onKeyPress(JOY_LEFT, goLeft);
        KOS::onKeyPress(JOY_RIGHT, goRight);

        while(true) {
            drawGrid();
            moveSnake();
            
            for(int i = 1; i < snake.size(); i++) {
                if(snake[0].x == snake[i].x and snake[0].y == snake[i].y) {
                    gameOver();
                }
            }
            if(snake[0].x < 0 or snake[0].y < 0 or snake[0].x >= fieldw or snake[0].y >= fieldh) gameOver();

            if(snake[0].x == apple.x and snake[0].y == apple.y) {
                apple.x = random(0, fieldw);
                apple.y = random(0, fieldh);
                appleEaten = true;
            }
            canvas.pushImage(apple.x*20, apple.y*20, 20, 20, (lgfx::rgb565_t *) appleImg, TFT_BLACK);
            drawSnake();
            // sendInfo();
            
            canvas.pushSprite(0, 0);
            vTaskDelay(300);
        }
    }

    void init() {
        xTaskCreatePinnedToCore(
            main,
            "Zmeyka task",
            16384,
            NULL,
            1,
            NULL, 
            1
        );
    }
}