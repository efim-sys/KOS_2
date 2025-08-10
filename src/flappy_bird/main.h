#include <vector>

namespace FlappyBird {
    uint16_t * background;

    uint16_t * textures_bird[3];

    uint16_t * pipe;
    uint16_t * pipe2;

    float worldSpeed = 1;

    struct {
        double vspeed, x, y;

        uint8_t wing_frame = 0;
    } bird;

    struct Pipe {
        int x, y;

        int height;

        bool done;
    };

    uint32_t frame = 0;

    float worldx = 0;

    int score = 0;

    std::vector<Pipe> pipes;

    void reset() {
        frame = 0;
        worldx = 0;

        worldSpeed = 1;

        pipes = {
            Pipe{200, 120, 100, false}, 
            Pipe{350, 60, 100, false},
            Pipe{500, 80, 100, false},
            Pipe{650, 140, 100, false},
            Pipe{800, 90, 100, false},
            Pipe{950, 30, 100, false},
        };


        score = 0;

        bird.vspeed = 0;
        bird.x = 60;
        bird.y = 120;
        bird.wing_frame = 0;
    }

    bool AImode = false;

    void main(void* arg) {
        AImode = !digitalRead(0);

        KOS::initSPIFFS();

        background = KOS::readImageBmp(SPIFFS, "/FB_background.bmp");

        textures_bird[0] = KOS::readImageBmp(SPIFFS, "/FB_bird1.bmp");
        textures_bird[1] = KOS::readImageBmp(SPIFFS, "/FB_bird2.bmp");
        textures_bird[2] = KOS::readImageBmp(SPIFFS, "/FB_bird3.bmp");

        pipe = KOS::readImageBmp(SPIFFS, "/FB_pipe.bmp");

        pipe2 = (uint16_t *) ps_malloc(52*200*2);

        memcpy(pipe2, pipe, 52*200*2);

        KOS::flipImageV(pipe2, 52, 200);

        canvas.pushImage(0, 0, 240, 240, background);

        // for(int i = 0; i < 3; i ++) {
        //     // canvas.pushImageRotateZoom(0, 50*i, 34, 24, textures_bird[i], 0xE007);

        //     canvas.pushImageRotateZoom(50, 50+50*i, 34/2, 24/4, -45+i*45, 1.5, 1.5, 34, 24, textures_bird[i], 0xE007);
        // }

        KOS::onKeyPress(BTN_UP, [](uint8_t k) {
            bird.vspeed = 5;
        });

        
        reset();
        
        
        
        

        while(true) {
            frame ++;
            canvas.clear(TFT_DARKGREEN);
            canvas.pushImage(0, 0, 240, 240, background);
            
            float angle = 0 + constrain(bird.vspeed*-3, -80, 80);

            // canvas.fillRect(bird.x, 240-bird.y, 34, 24, TFT_RED);

            canvas.pushImageRotateZoom(bird.x+34/2, canvas.height()-bird.y+6, 34/2, 24/4, angle, 1, 1, 34, 24, textures_bird[bird.wing_frame], 0xE007);

            bool isDead = false;

            bool deletePipe = false;

            for(int i = 0; i < pipes.size(); i ++) {
                Pipe * pp = &pipes[i];

                int realx = pp->x - worldx;

                if(realx < -52) deletePipe = true;

                if(realx > 240) continue;

                isDead |= (realx - 34 <= bird.x) and (realx + 52 >= bird.x) and ((pp->y > bird.y - 24) or (pp->y + pp->height < bird.y));

                if(!pp->done and realx < bird.x) {
                    pp->done = true;
                    score ++;
                    KOS::playSound(&SPIFFS, "/score.sound");
                }

                canvas.pushImage(realx, canvas.height()-pp->y, 52, 200, pipe, TFT_WHITE);
                canvas.pushImage(realx, canvas.height()-(pp->y+pp->height+200), 52, 200, pipe2, TFT_WHITE);
            }

            if(deletePipe) {
                pipes.erase(pipes.begin());

                pipes.push_back(Pipe{pipes.back().x+120, random(30, 120), constrain(pipes.back().height-1, 80, 200)});
            }

            

            

            bird.y += bird.vspeed;

            bird.vspeed -= 0.3;

            canvas.setTextColor(TFT_GOLD);
            canvas.setTextDatum(textdatum_t::top_left);

            canvas.drawString(String(score), 0, 0, &lgfx::fonts::Orbitron_Light_32);
            canvas.pushSprite(0, 0);

            worldx += worldSpeed;

            worldSpeed += 0.0005;

            if(frame%4 == 0) bird.wing_frame = (bird.wing_frame+1)%3;

            if(AImode) {
                Pipe* pp;

                for(int i = 0; i < pipes.size(); i++) {
                    pp = &pipes[i];

                    float realx = pp->x - worldx;

                    if (realx + 52 > bird.x) {
                        if(pp->y + 30 > bird.y) {
                            bird.vspeed = 5;
                        }
                        break;
                    }
                }
            }

            // delay(30);

            if(isDead) {
                // canvas.fillScreen(TFT_RED);

                canvas.pushSprite(0, 0);

                KOS::playSound(&SPIFFS, "/die.sound");

                delay(1000);

                reset();
            }
        }

        vTaskDelete(NULL);
    }

    void init() {
        xTaskCreatePinnedToCore(
            main,
            "FBird task",
            32768,
            NULL,
            1,
            NULL, 
            1
        );
    }
}