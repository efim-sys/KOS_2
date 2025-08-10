void intHandler0() {
    uint8_t thisID = 0;
    if (millis()-buttons[thisID].lastInterrupt < debounceMs) return;
    if (buttons[thisID].lastValue == buttons[thisID].inverted) {
        if(buttons[thisID].onKeyPress != nullptr) buttons[thisID].onKeyPress(thisID);
        KOS::autoSleep::lastButtonPress = millis();
        buttons[thisID].lastValue = !buttons[thisID].inverted;
    }
    else {
        if(buttons[thisID].onKeyRelease != nullptr) buttons[thisID].onKeyRelease(thisID);
        KOS::autoSleep::lastButtonPress = millis();
        buttons[thisID].lastValue = buttons[thisID].inverted;
    }
    
    buttons[thisID].lastInterrupt = millis();
}

void intHandler1() {
    uint8_t thisID = 1;
    if (millis()-buttons[thisID].lastInterrupt < debounceMs) return;
    if (buttons[thisID].lastValue == buttons[thisID].inverted) {
        if(buttons[thisID].onKeyPress != nullptr) buttons[thisID].onKeyPress(thisID);
        KOS::autoSleep::lastButtonPress = millis();
        buttons[thisID].lastValue = !buttons[thisID].inverted;
    }
    else {
        if(buttons[thisID].onKeyRelease != nullptr) buttons[thisID].onKeyRelease(thisID);
        KOS::autoSleep::lastButtonPress = millis();
        buttons[thisID].lastValue = buttons[thisID].inverted;
    }
    
    buttons[thisID].lastInterrupt = millis();
}

void intHandler2() {
    uint8_t thisID = 2;
    if (millis()-buttons[thisID].lastInterrupt < debounceMs) return;
    if (buttons[thisID].lastValue == buttons[thisID].inverted) {
        if(buttons[thisID].onKeyPress != nullptr) buttons[thisID].onKeyPress(thisID);
        KOS::autoSleep::lastButtonPress = millis();
        buttons[thisID].lastValue = !buttons[thisID].inverted;
    }
    else {
        if(buttons[thisID].onKeyRelease != nullptr) buttons[thisID].onKeyRelease(thisID);
        KOS::autoSleep::lastButtonPress = millis();
        buttons[thisID].lastValue = buttons[thisID].inverted;
    }
    
    buttons[thisID].lastInterrupt = millis();
}

void intHandler3() {
    uint8_t thisID = 3;
    if (millis()-buttons[thisID].lastInterrupt < debounceMs) return;
    if (buttons[thisID].lastValue == buttons[thisID].inverted) {
        if(buttons[thisID].onKeyPress != nullptr) buttons[thisID].onKeyPress(thisID);
        KOS::autoSleep::lastButtonPress = millis();
        buttons[thisID].lastValue = !buttons[thisID].inverted;
    }
    else {
        if(buttons[thisID].onKeyRelease != nullptr) buttons[thisID].onKeyRelease(thisID);
        KOS::autoSleep::lastButtonPress = millis();
        buttons[thisID].lastValue = buttons[thisID].inverted;
    }
    
    buttons[thisID].lastInterrupt = millis();
}

void intHandler4() {
    uint8_t thisID = 4;
    if (millis()-buttons[thisID].lastInterrupt < debounceMs) return;
    if (buttons[thisID].lastValue == buttons[thisID].inverted) {
        if(buttons[thisID].onKeyPress != nullptr) buttons[thisID].onKeyPress(thisID);
        KOS::autoSleep::lastButtonPress = millis();
        buttons[thisID].lastValue = !buttons[thisID].inverted;
    }
    else {
        if(buttons[thisID].onKeyRelease != nullptr) buttons[thisID].onKeyRelease(thisID);
        KOS::autoSleep::lastButtonPress = millis();
        buttons[thisID].lastValue = buttons[thisID].inverted;
    }
    
    buttons[thisID].lastInterrupt = millis();
}

void intHandler5() {
    uint8_t thisID = 5;
    if (millis()-buttons[thisID].lastInterrupt < debounceMs) return;
    if (buttons[thisID].lastValue == buttons[thisID].inverted) {
        if(buttons[thisID].onKeyPress != nullptr) buttons[thisID].onKeyPress(thisID);
        KOS::autoSleep::lastButtonPress = millis();
        buttons[thisID].lastValue = !buttons[thisID].inverted;
    }
    else {
        if(buttons[thisID].onKeyRelease != nullptr) buttons[thisID].onKeyRelease(thisID);
        KOS::autoSleep::lastButtonPress = millis();
        buttons[thisID].lastValue = buttons[thisID].inverted;
    }
    
    buttons[thisID].lastInterrupt = millis();
}

void intHandler6() {
    uint8_t thisID = 6;
    if (millis()-buttons[thisID].lastInterrupt < debounceMs) return;
    if (buttons[thisID].lastValue == buttons[thisID].inverted) {
        if(buttons[thisID].onKeyPress != nullptr) buttons[thisID].onKeyPress(thisID);
        KOS::autoSleep::lastButtonPress = millis();
        buttons[thisID].lastValue = !buttons[thisID].inverted;
    }
    else {
        if(buttons[thisID].onKeyRelease != nullptr) buttons[thisID].onKeyRelease(thisID);
        KOS::autoSleep::lastButtonPress = millis();
        buttons[thisID].lastValue = buttons[thisID].inverted;
    }
    
    buttons[thisID].lastInterrupt = millis();
}

void intHandler7() {
    uint8_t thisID = 7;
    if (millis()-buttons[thisID].lastInterrupt < debounceMs) return;
    if (buttons[thisID].lastValue == buttons[thisID].inverted) {
        if(buttons[thisID].onKeyPress != nullptr) buttons[thisID].onKeyPress(thisID);
        KOS::autoSleep::lastButtonPress = millis();
        buttons[thisID].lastValue = !buttons[thisID].inverted;
    }
    else {
        if(buttons[thisID].onKeyRelease != nullptr) buttons[thisID].onKeyRelease(thisID);
        KOS::autoSleep::lastButtonPress = millis();
        buttons[thisID].lastValue = buttons[thisID].inverted;
    }
    
    buttons[thisID].lastInterrupt = millis();
}