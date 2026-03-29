#include "KOS/KOS.h"
#include "KUI/KUI.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID "00001000-1212-efde-1523-785feabcd123"
#define CHARACTERISTIC_UUID "00001001-1212-efde-1523-785feabcd123"

static BLEUUID serviceUUID(SERVICE_UUID);
static BLEUUID    charUUID(CHARACTERISTIC_UUID);

#define LED_PIN 45

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint8_t txCounter = 0xB9;

enum
{
    SELECT_NONE = 0,
    SELECT_SENDER = 1,
    SELECT_RECEIVER = 2
};

int selection = SELECT_NONE;
SemaphoreHandle_t select_sm = xSemaphoreCreateBinary(); // Изначально занято

void set_sender()
{
    selection = SELECT_SENDER;
    xSemaphoreGive(select_sm);
}

void set_receiver()
{
    selection = SELECT_RECEIVER;
    xSemaphoreGive(select_sm);
}

int targetMgdl = 126;

void set_sender_menu() {
    KUI::window = {
        KUI::Element(ELEMENT_TEXT, "Sender is ready...\nWaiting for connection...", NULL, TFT_WHITE, NULL),
        KUI::Element(ELEMENT_TEXT, "Counter=0x" + String(txCounter, 16), NULL, TFT_CYAN, NULL),
        KUI::Element(ELEMENT_TEXT, "Sugar=" + String(targetMgdl) + "mg/dL", NULL, TFT_GREEN, NULL),

        KUI::Element(ELEMENT_BUTTON, "+", NULL, TFT_GREEN, [](){ targetMgdl++; set_sender_menu();}),
        KUI::Element(ELEMENT_BUTTON, "-", NULL, TFT_RED, [](){ targetMgdl--;  set_sender_menu();}),
    };
    KUI::requestWindowUpdate();
}

void CGM_sender()
{
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    BLEDevice::init("SN1740005690");
    BLEServer *pServer = BLEDevice::createServer();

    // Коллбэки для статуса подключения
    class MyServerCallbacks : public BLEServerCallbacks
    {
        void onConnect(BLEServer *pServer) { deviceConnected = true; }
        void onDisconnect(BLEServer *pServer)
        {
            deviceConnected = false;
            BLEDevice::startAdvertising(); // Перезапуск рекламы при разрыве
        }
    };
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Создаем характеристику
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_INDICATE);

    // ВАЖНО: Добавляем дескриптор 0x2902, чтобы в nRF появились "стрелочки"
    pCharacteristic->addDescriptor(new BLE2902());

    pService->start();

    // Настройка рекламы (Advertising)
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();

    BLEAdvertisementData oAdvData;
    oAdvData.setFlags(0x06);
    oAdvData.setManufacturerData("\x43\x47\x01\x02");
    oAdvData.setName("SN1740005690");

    pAdvertising->setAdvertisementData(oAdvData);

    BLEAdvertisementData oScanRes;
    oScanRes.setCompleteServices(BLEUUID(SERVICE_UUID));
    pAdvertising->setScanResponseData(oScanRes);

    pAdvertising->setMinInterval(800); 
    pAdvertising->setMaxInterval(800); 

    pAdvertising->start();

    KUI::window = {KUI::Element(ELEMENT_TEXT, "Sender is ready...\nWaiting for connection..", NULL, TFT_WHITE, NULL)};
    KUI::requestWindowUpdate();

    while (1)
    {
        if (deviceConnected)
        {
            set_sender_menu();

            uint8_t glucoseByte = (targetMgdl - 20) / 2;

            // Формируем пакет из 12 байт
            uint8_t packet[12] = {
                0x07,             // [0] Маркер
                txCounter++,      // [1] Счетчик пакетов
                0x01,             // [2] Статичный байт
                0x01,             // [3] Статичный байт
                glucoseByte,      // [4] ВАШ САХАР (Байт 5)
                0x07,             // [5] Доп. данные (Байт 6)
                0x4A, 0x46, 0x4C, // [6,7,8] Статичные маркеры
                0x04, 0x1C,       // [9,10] Концевые маркеры
                0x48              // [11] Чексумма (пока статичная)
            };

            // Мигаем светодиодом при отправке
            digitalWrite(LED_PIN, HIGH);

            pCharacteristic->setValue(packet, 12);
            pCharacteristic->notify(); // Отправка уведомления

            USBSerial.printf("Данные отправлены! Счетчик: %02X\n", packet[1]);

            delay(200); // Короткая вспышка
            digitalWrite(LED_PIN, LOW);

            delay(4800); // Пауза 5 секунд между пакетами (для теста)
        }
        else
        {
            KUI::window = {KUI::Element(ELEMENT_TEXT, "Sender is ready...\nWaiting for connection...", NULL, TFT_WHITE, NULL)};
            KUI::requestWindowUpdate();
        }
    }
}

#define MAX_SCAN_DEVICES 100

SemaphoreHandle_t xConnectSemaphore = NULL;
int selectedDeviceIdx = -1;
int foundCount = 0;

BLEAdvertisedDevice* foundDevices[MAX_SCAN_DEVICES]; 
int lastSugar = 0;
bool isConnecting = false;

void set_receiver_active_menu(String devName) {
    KUI::window = {
        KUI::Element(ELEMENT_TEXT, "Sensor: " + devName, NULL, TFT_CYAN, NULL),
        KUI::Element(ELEMENT_TEXT, "Sugar: " + String(lastSugar) + " mg/dL", NULL, (lastSugar > 180) ? TFT_RED : TFT_GREEN, NULL),
        KUI::Element(ELEMENT_TEXT, "Value: " + String(lastSugar / 18.1, 1) + " mmol/L", NULL, TFT_YELLOW, NULL),
        KUI::Element(ELEMENT_BUTTON, "BACK / DISCONNECT", NULL, TFT_WHITE, [](){ ESP.restart(); }) 
    };
    KUI::requestWindowUpdate();
}

void onDeviceSelect() {
    // KUI::activeElement - 1, так как 0-й элемент это заголовок TEXT
    selectedDeviceIdx = KUI::activeElement - 1;
    
    if (xConnectSemaphore != NULL) {
        xSemaphoreGive(xConnectSemaphore); // Даем сигнал на подключение
    }
}

void CGM_receiver() {
    // Создаем пустой бинарный семафор
    if (xConnectSemaphore == NULL) {
        xConnectSemaphore = xSemaphoreCreateBinary();
    }

    BLEDevice::init("S3_CGM_Client");
    
    // ЭКРАН 1: Поиск
    KUI::window = { KUI::Element(ELEMENT_TEXT, "Searching for PocTech...", NULL, TFT_WHITE, NULL) };
    KUI::requestWindowUpdate();

    BLEScan* pScan = BLEDevice::getScan();
    pScan->setActiveScan(true);
    BLEScanResults* pResults = pScan->start(5, false);
    
    int total = pResults->getCount();
    foundCount = 0;

    // ЭКРАН 2: Список (Фильтрация)
    KUI::window = { KUI::Element(ELEMENT_TEXT, "Select Sensor:", NULL, TFT_YELLOW, NULL) };
    
    for (int i = 0; i < total && foundCount < MAX_SCAN_DEVICES; i++) {
        BLEAdvertisedDevice d = pResults->getDevice(i);
        String mData = d.getManufacturerData();

        if (mData.length() >= 2 && (uint8_t)mData[0] == 0x43 && (uint8_t)mData[1] == 0x47) {
            foundDevices[foundCount] = new BLEAdvertisedDevice(d);
            String name = d.getName().c_str();
            if (name == "") name = d.getAddress().toString().c_str();

            // Кнопка просто вызывает семафор
            KUI::window.push_back(KUI::Element(ELEMENT_BUTTON, name, NULL, TFT_WHITE, onDeviceSelect));
            foundCount++;
        }
    }
    KUI::requestWindowUpdate();

    // ОЖИДАНИЕ ВЫБОРА (Блокировка потока до нажатия кнопки)
    if (xSemaphoreTake(xConnectSemaphore, portMAX_DELAY) == pdTRUE) {
        if (selectedDeviceIdx >= 0 && selectedDeviceIdx < foundCount) {
            
            String dName = foundDevices[selectedDeviceIdx]->getName().c_str();
            KUI::window = { KUI::Element(ELEMENT_TEXT, "Connecting...", NULL, TFT_CYAN, NULL) };
            KUI::requestWindowUpdate();

            BLEClient* pClient = BLEDevice::createClient();
            if (pClient->connect(foundDevices[selectedDeviceIdx])) {
                BLERemoteService* pSvc = pClient->getService(serviceUUID);
                if (pSvc) {
                    BLERemoteCharacteristic* pCr = pSvc->getCharacteristic(charUUID);
                    if (pCr && pCr->canNotify()) {
                        pCr->registerForNotify([](BLERemoteCharacteristic* c, uint8_t* d, size_t l, bool n) {
                            if (l >= 5) lastSugar = (d[4] * 2) + 20;
                        });

                        // ЦИКЛ ПРИЕМА (Теперь интерфейс не виснет, так как мы в основном потоке функции)
                        while(pClient->isConnected()) {
                            set_receiver_active_menu(dName);
                            delay(1000);
                        }
                    }
                }
            }
            Serial.println("Disconnected. Restarting...");
            delay(2000);
            ESP.restart();
        }
    }
}



void CGM_main(void *arg)
{
    KUI::window = {
        KUI::Element(ELEMENT_BUTTON, "Sender", NULL, TFT_YELLOW, set_sender),
        KUI::Element(ELEMENT_BUTTON, "Receiver", NULL, TFT_GREEN, set_receiver),
    };

    KUI::initWindow();

    KOS::autoSleep::enable = false;

    xSemaphoreTake(select_sm, portMAX_DELAY); // Ожидать освобождения семафора

    switch (selection)
    {
    case SELECT_RECEIVER:
        CGM_receiver();
        break;
    case SELECT_SENDER:
        CGM_sender();
        break;
    }
}

void CGM_init()
{
    xTaskCreate(CGM_main, "CGM_app", 65536, NULL, 5, NULL);
}