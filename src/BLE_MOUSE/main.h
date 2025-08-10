// namespace BLE_MOUSE {
//     #include <BleMouse.h>
//     BleMouse bleMouse;

//     int DPI = 20;

//     void main(void *p) {
//         // bleMouse.deviceName = "Korobochka mouse";
//         bleMouse.batteryLevel = constrain(KOS::getBattery(), 0, 1) * 100;

//         bleMouse.begin();

//         while(true) {
//             int8_t dx = digitalRead(buttons[JOY_RIGHT].pin) - digitalRead(buttons[JOY_LEFT].pin);
//             int8_t dy = digitalRead(buttons[JOY_UP].pin) - digitalRead(buttons[JOY_DOWN].pin);

//             dx*=DPI;
//             dy*=DPI;

//             bleMouse.move(dx, dy);

//             vTaskDelay(50);
//         }
//     }

//     void init() {
//         xTaskCreate(main, "BLE_MS", 64000, NULL, 2, NULL);
//     }
// }