#include <USBMSC.h>

namespace cardReader {
    USBMSC msc;

    uint32_t totalRed, totalWrote = 0;

    static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize) {
        uint32_t secSize = SD_MMC.sectorSize();
        if (!secSize) {
          return false;  // disk error
        }
        for (int x = 0; x < bufsize / secSize; x++) {
          uint8_t blkbuffer[secSize];
          memcpy(blkbuffer, (uint8_t *)buffer + secSize * x, secSize);
          totalWrote += secSize;
          if (!SD_MMC.writeRAW(blkbuffer, lba + x)) {
            return false;
          }
        }
        return bufsize;
    }
      
    static int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) {
        uint32_t secSize = SD_MMC.sectorSize();
        if (!secSize) {
          return false;  // disk error
        }
        // log_v("Read lba: %ld\toffset: %ld\tbufsize: %ld\tsector: %lu", lba, offset, bufsize, secSize);
        for (int x = 0; x < bufsize / secSize; x++) {
          totalRed += secSize;
          if (!SD_MMC.readRAW((uint8_t *)buffer + (x * secSize), lba + x)) {
            return false;  // outside of volume boundary
          }
        }
        return bufsize;
    }


    static bool onStartStop(uint8_t power_condition, bool start, bool load_eject) {
      log_i("Start/Stop power: %u\tstart: %d\teject: %d", power_condition, start, load_eject);
      KOS::playSound(&SPIFFS, "/score.sound");
      return true;
    }

    void main (void* p) {
      KOS::initSD();

      msc.vendorID("ESP32");
      msc.productID("USB_MSC");
      msc.productRevision("1.0");
      msc.onRead(onRead);
      msc.onWrite(onWrite);
      msc.onStartStop(onStartStop);
      msc.mediaPresent(true);
      msc.begin(SD_MMC.numSectors(), SD_MMC.sectorSize());

      char buffer[64];

      snprintf(buffer, 64, "Used %.3f MB of %.3f MB", SD_MMC.usedBytes()/(1024.0*1024.0), SD_MMC.cardSize()/(1024.0*1024.0));

      KUI::window={
        KUI::Element(ELEMENT_TEXT, "\nYour korobochka is in micro sd card reader mode\nInsert SD card and use its files on PC via USB cable!", NULL, TFT_GREENYELLOW, NULL),
        KUI::Element(ELEMENT_TEXT, String(buffer), NULL, TFT_GREENYELLOW, NULL),
        KUI::Element(ELEMENT_TEXT, "Total read: 0 MB", NULL, TFT_GREENYELLOW, NULL),
        KUI::Element(ELEMENT_TEXT, "Total write: 0 MB", NULL, TFT_GREENYELLOW, NULL),
      };

      KUI::initWindow();

      KOS::autoSleep::enable = false;

      
      vTaskDelete(NULL);
      // while(true) {
      //   KUI::window[2].text = String(std::format("Total read: %.3f MB"))
      // }
    }

    void init() {
      xTaskCreate(main, "SD card reader", 65536, NULL, 5, NULL);
    }
}