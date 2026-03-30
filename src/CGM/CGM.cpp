#include "KOS/KOS.h"
#include "KUI/KUI.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID "00001000-1212-efde-1523-785feabcd123"
#define CHARACTERISTIC_UUID "00001001-1212-efde-1523-785feabcd123"
#define CCCD_UUID "00002902-0000-1000-8000-00805f9b34fb"

static BLEUUID serviceUUID(SERVICE_UUID);
static BLEUUID charUUID(CHARACTERISTIC_UUID);
static BLEUUID cccdUUID(CCCD_UUID);

#define LED_PIN 45

// ======================== Общие переменные ========================
enum { SELECT_NONE = 0, SELECT_SENDER = 1, SELECT_RECEIVER = 2 };
int selection = SELECT_NONE;
SemaphoreHandle_t select_sm = xSemaphoreCreateBinary();

int targetMgdl = 126;   // для Sender

// ======================== Sender ========================
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint8_t txCounter = 0xB9;

void set_sender_menu() {
    KUI::window = {
        KUI::Element(ELEMENT_TEXT, "Sender is ready...\nWaiting for connection...", NULL, TFT_WHITE, NULL),
        KUI::Element(ELEMENT_TEXT, "Counter=0x" + String(txCounter, 16), NULL, TFT_CYAN, NULL),
        KUI::Element(ELEMENT_TEXT, "Sugar=" + String(targetMgdl) + "mg/dL", NULL, TFT_GREEN, NULL),
        KUI::Element(ELEMENT_BUTTON, "+", NULL, TFT_GREEN, [](){ targetMgdl++; set_sender_menu(); }),
        KUI::Element(ELEMENT_BUTTON, "-", NULL, TFT_RED, [](){ targetMgdl--; set_sender_menu(); }),
        KUI::Element(ELEMENT_SWITCH, "Autosleep", NULL, TFT_GOLD, NULL, &fonts::DejaVu18, &KOS::autoSleep::enable)
    };
    KUI::requestWindowUpdate();
}

void CGM_sender() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    char sn[16];
    uint32_t randNum = esp_random();
    snprintf(sn, sizeof(sn), "SN%010u", (unsigned int)randNum);
    BLEDevice::init(sn);
    USBSerial.printf("Device started with SN: %s\n", sn);

    BLEServer *pServer = BLEDevice::createServer();
    class MyServerCallbacks : public BLEServerCallbacks {
        void onConnect(BLEServer *pServer) { deviceConnected = true; }
        void onDisconnect(BLEServer *pServer) {
            deviceConnected = false;
            BLEDevice::startAdvertising();
        }
    };
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(serviceUUID);
    pCharacteristic = pService->createCharacteristic(
        charUUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE);
    pCharacteristic->addDescriptor(new BLE2902());
    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    BLEAdvertisementData oAdvData;
    oAdvData.setFlags(0x06);
    oAdvData.setManufacturerData("\x43\x47\x01\x02");
    oAdvData.setName(sn);
    pAdvertising->setAdvertisementData(oAdvData);

    BLEAdvertisementData oScanRes;
    oScanRes.setCompleteServices(serviceUUID);
    pAdvertising->setScanResponseData(oScanRes);
    pAdvertising->setMinInterval(800);
    pAdvertising->setMaxInterval(800);
    pAdvertising->start();

    KUI::window = { KUI::Element(ELEMENT_TEXT, "Sender is ready...\nWaiting for connection...", NULL, TFT_WHITE, NULL) };
    KUI::requestWindowUpdate();

    while (1) {
        if (deviceConnected) {
            set_sender_menu();

            uint8_t glucoseByte = (targetMgdl - 20) / 2;
            uint8_t packet[12] = {
                0x07, txCounter++, 0x01, 0x01,
                glucoseByte, 0x07, 0x4A, 0x46, 0x4C, 0x04, 0x1C, 0x48
            };

            digitalWrite(LED_PIN, HIGH);
            pCharacteristic->setValue(packet, 12);
            pCharacteristic->notify();
            USBSerial.printf("Data sent! Counter: %02X\n", packet[1]);

            delay(200);
            digitalWrite(LED_PIN, LOW);
            delay(4800);
        } else {
            KUI::window = { KUI::Element(ELEMENT_TEXT, "Sender is ready...\nWaiting for connection...", NULL, TFT_WHITE, NULL) };
            KUI::requestWindowUpdate();
            delay(100);
        }
    }
}

#define MAX_SCAN_DEVICES 100   // увеличено для хранения всех устройств

struct DeviceInfo {
    String addressStr;
    String name;
    bool isCGM;
    int rssi;                   // добавлен для возможного отображения
};

DeviceInfo devices[MAX_SCAN_DEVICES];
int deviceCount = 0;            // переименовано вместо cgmCount

SemaphoreHandle_t xDeviceSelectedSemaphore = NULL;
SemaphoreHandle_t xRescanSemaphore = NULL;
int selectedDeviceIdx = -1;
bool rescanRequested = false;
volatile int lastSugar = 0;

void onDeviceSelect() {
    selectedDeviceIdx = KUI::activeElement - 1;
    if (xDeviceSelectedSemaphore != NULL) {
        xSemaphoreGive(xDeviceSelectedSemaphore);
    }
}

void onRescan() {
    rescanRequested = true;
    if (xRescanSemaphore != NULL) {
        xSemaphoreGive(xRescanSemaphore);
    }
}

int lastRSSI = 0;   // хранит RSSI текущего подключённого устройства

void buildDeviceList() {
    KUI::window.clear();
    KUI::window.push_back(KUI::Element(ELEMENT_TEXT, "Select BLE device:", NULL, TFT_YELLOW, NULL));

    if (deviceCount == 0) {
        KUI::window.push_back(KUI::Element(ELEMENT_TEXT, "No devices found", NULL, TFT_RED, NULL));
    } else {
        for (int i = 0; i < deviceCount; i++) {
            String displayName = devices[i].name;
            if (displayName == "") displayName = devices[i].addressStr;
            if (devices[i].isCGM) {
                displayName += " [CGM]";
            }
            // Цвет: зелёный для CGM, белый для остальных
            uint16_t btnColor = devices[i].isCGM ? TFT_GREEN : TFT_WHITE;
            KUI::window.push_back(KUI::Element(ELEMENT_BUTTON, displayName, NULL, btnColor, onDeviceSelect));
        }
    }
    KUI::window.push_back(KUI::Element(ELEMENT_BUTTON, "RESCAN", NULL, TFT_ORANGE, onRescan));
    KUI::requestWindowUpdate();
}

void performScan() {
    deviceCount = 0;
    KUI::window = { KUI::Element(ELEMENT_TEXT, "Scanning for 5 seconds...", NULL, TFT_WHITE, NULL) };
    KUI::requestWindowUpdate();

    BLEScan* pScan = BLEDevice::getScan();
    pScan->setActiveScan(true);
    BLEScanResults* pResults = pScan->start(5, false);
    int total = pResults->getCount();

    for (int i = 0; i < total && deviceCount < MAX_SCAN_DEVICES; i++) {
        BLEAdvertisedDevice d = pResults->getDevice(i);
        String mData = d.getManufacturerData();

        // Проверяем, является ли устройство CGM по manufacturer data
        bool isCGM = (mData.length() >= 2 && (uint8_t)mData[0] == 0x43 && (uint8_t)mData[1] == 0x47);

        devices[deviceCount].addressStr = d.getAddress().toString().c_str();
        devices[deviceCount].name = d.getName().c_str();
        devices[deviceCount].isCGM = isCGM;
        devices[deviceCount].rssi = d.getRSSI();
        deviceCount++;
    }

    // Освобождаем память результатов (если требуется)
    // delete pResults;
    buildDeviceList();
}

void enableNotify(BLERemoteCharacteristic* pRemoteChar) {
    if (pRemoteChar->canNotify()) {
        BLERemoteDescriptor* pDescriptor = pRemoteChar->getDescriptor(cccdUUID);
        if (pDescriptor != nullptr) {
            uint8_t cccdValue[] = {0x01, 0x00};
            pDescriptor->writeValue(cccdValue, 2);
            USBSerial.println("CCCD written to enable notifications");
        } else {
            USBSerial.println("Failed to get CCCD descriptor");
        }
    }
}

KUI::Canvas plot;

void set_receiver_active_menu(String devName) {
    KUI::window = {
        KUI::Element(ELEMENT_TEXT, "Sensor: " + devName, NULL, TFT_CYAN, NULL),
        KUI::Element(ELEMENT_TEXT, "Sugar: " + String(lastSugar) + " mg/dL", NULL, (lastSugar > 180) ? TFT_RED : TFT_GREEN, NULL),
        KUI::Element(ELEMENT_TEXT, "Value: " + String(lastSugar / 18.1, 1) + " mmol/L", NULL, TFT_YELLOW, NULL),
        KUI::Element(ELEMENT_TEXT, "RSSI: " + String(lastRSSI) + " dBm", NULL, TFT_WHITE, NULL),   // новое поле
        KUI::Element(ELEMENT_BUTTON, "BACK / DISCONNECT", NULL, TFT_WHITE, [](){ ESP.restart(); }),
        KUI::Element(ELEMENT_SWITCH, "Autosleep", NULL, TFT_GOLD, NULL, &fonts::DejaVu18, &KOS::autoSleep::enable),
    };
    KUI::requestWindowUpdate();
}

void CGM_receiver() {
    if (xDeviceSelectedSemaphore == NULL) {
        xDeviceSelectedSemaphore = xSemaphoreCreateBinary();
    }
    if (xRescanSemaphore == NULL) {
        xRescanSemaphore = xSemaphoreCreateBinary();
    }

    BLEDevice::init("S3_CGM_Client");

    while (true) {
        performScan();

        while (true) {
            if (xSemaphoreTake(xDeviceSelectedSemaphore, 0) == pdTRUE) {
                if (selectedDeviceIdx >= 0 && selectedDeviceIdx < deviceCount) {
                    // Если выбрано не CGM – выводим предупреждение, но продолжаем
                    if (!devices[selectedDeviceIdx].isCGM) {
                        USBSerial.println("Warning: Selected device is not a CGM sensor. Attempting to connect anyway...");
                        KUI::window = { KUI::Element(ELEMENT_TEXT, "Warning: Not a CGM device!\nTrying to connect...", NULL, TFT_RED, NULL) };
                        KUI::requestWindowUpdate();
                        delay(2000);
                        // Не прерываем выполнение, идём на подключение
                    }
                    break;
                }
                performScan();
                continue;
            }
            if (xSemaphoreTake(xRescanSemaphore, 0) == pdTRUE) {
                if (rescanRequested) {
                    rescanRequested = false;
                    performScan();
                }
                continue;
            }
            delay(100);
        }

        String devName = devices[selectedDeviceIdx].name;
        if (devName == "") devName = devices[selectedDeviceIdx].addressStr;
        String devAddressStr = devices[selectedDeviceIdx].addressStr;

        // После выбора устройства сохраняем его RSSI
        lastRSSI = devices[selectedDeviceIdx].rssi;

        KUI::activeElement = 0;
        KUI::scrollY = 0;

        KUI::window = { KUI::Element(ELEMENT_TEXT, "Connecting to " + devName + "...", NULL, TFT_CYAN, NULL) };
        KUI::requestWindowUpdate();

        BLEClient* pClient = BLEDevice::createClient();
        BLEAddress devAddress(devAddressStr.c_str());
        if (!pClient->connect(devAddress)) {
            USBSerial.println("Connection failed");
            delay(2000);
            continue;
        }

        BLERemoteService* pSvc = pClient->getService(serviceUUID);
        if (!pSvc) {
            USBSerial.println("Service not found");
            pClient->disconnect();
            delay(2000);
            continue;
        }

        BLERemoteCharacteristic* pCr = pSvc->getCharacteristic(charUUID);
        if (!pCr || !pCr->canNotify()) {
            USBSerial.println("Characteristic not suitable for notify");
            pClient->disconnect();
            delay(2000);
            continue;
        }

        enableNotify(pCr);

        // Добавляем запись CCCD для второй характеристики (Service Changed)
        BLERemoteCharacteristic* pServiceChanged = pSvc->getCharacteristic(BLEUUID("00002a05-0000-1000-8000-00805f9b34fb"));
        if (pServiceChanged && pServiceChanged->canNotify()) {
            BLERemoteDescriptor* pDesc = pServiceChanged->getDescriptor(BLEUUID((uint16_t)0x2902));
            if (pDesc) {
                uint8_t val[] = {0x01, 0x00};
                pDesc->writeValue(val, 2);
                USBSerial.println("CCCD written to Service Changed characteristic");
            } else {
                USBSerial.println("Failed to get CCCD for Service Changed");
            }
        } else {
            USBSerial.println("Service Changed characteristic not found or cannot notify");
        }

        pCr->registerForNotify([](BLERemoteCharacteristic* c, uint8_t* d, size_t l, bool n) {
            if (l >= 5 && d[0] == 0x07) {
                lastSugar = (d[4] * 2) + 20;
            }
        });

        while (pClient->isConnected()) {
            set_receiver_active_menu(devName);
            delay(1000);
        }

        USBSerial.println("Disconnected. Re-scanning...");
        delay(2000);
    }
}

// ======================== Главная задача ========================
void CGM_main(void *arg) {
    plot.data = KOS::readImageBmp(SPIFFS, "/wall.bmp", (uint32_t*) &plot.width, (uint32_t*) &plot.height);
    USBSerial.printf("data=%p width = %d height=%d\n\n", plot.data, plot.width, plot.height);

    KUI::window = {
        KUI::Element(ELEMENT_BUTTON, "Sender", NULL, TFT_YELLOW, [](){ selection = SELECT_SENDER; xSemaphoreGive(select_sm); }),
        KUI::Element(ELEMENT_BUTTON, "Receiver", NULL, TFT_GREEN, [](){ selection = SELECT_RECEIVER; xSemaphoreGive(select_sm); }),
        KUI::Element(ELEMENT_IMAGE, "", &plot, 0, NULL)
    };
    KUI::initWindow();
    KOS::autoSleep::enable = false;

    xSemaphoreTake(select_sm, portMAX_DELAY);

    switch (selection) {
        case SELECT_RECEIVER: CGM_receiver(); break;
        case SELECT_SENDER:   CGM_sender();   break;
    }
}

void CGM_init() {
    xTaskCreate(CGM_main, "CGM_app", 65536, NULL, 5, NULL);
}