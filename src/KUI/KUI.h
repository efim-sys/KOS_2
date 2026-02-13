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
    struct Element {
        int8_t type = 0;

        String text;
        uint16_t* image;
        uint16_t color = TFT_MAGENTA;

        const lgfx::v1::GFXfont* font = &fonts::DejaVu18;

        void (*onClick)() = NULL;

        bool virtualSwitch = false;

        bool *switchPointer = NULL;

        Element(int8_t _type, String _text, uint16_t* _image, uint16_t _color, void (*_onClick)(), const lgfx::v1::GFXfont* _font = &fonts::DejaVu18, bool* swPtr = NULL ) {
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

