#pragma once

#include <vector>   
#include "KOS/KOS.h"

enum {
    ELEMENT_BUTTON = 0,
    ELEMENT_TEXT,
    ELEMENT_IMAGE,
    ELEMENT_SWITCH,
    ELEMENT_SCROLLER
};



namespace KUI {

    typedef struct {
        uint16_t *data;      /* массив пикселей в формате RGB565 */
        int width;           /* ширина в пикселях */
        int height;          /* высота в пикселях */
    } Canvas;

    struct Element {
        int8_t type = 0;

        String text;
        Canvas* image;
        uint16_t color = TFT_MAGENTA;

        const lgfx::v1::GFXfont* font = &fonts::DejaVu18;

        void (*onClick)() = NULL;

        bool virtualSwitch = false;

        bool *switchPointer = NULL;

        Element(int8_t _type, String _text, Canvas* _image, uint16_t _color, void (*_onClick)() = NULL, const lgfx::v1::GFXfont* _font = &fonts::DejaVu18, bool* swPtr = NULL ) {
            type = _type;
            text = _text;
            image = _image;
            color = _color;
            font = _font;
            onClick = _onClick;
            switchPointer = swPtr;
            if(switchPointer!=NULL) virtualSwitch = *switchPointer;
        }
    };

    extern std::vector<Element> window;

    extern int activeElement;

    extern int scrollY;

    void requestWindowUpdate();

    void initWindow();

    void terminateWindow(bool resetPosition = true);

};

