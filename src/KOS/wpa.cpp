#include "KOS.h"

namespace KOS {
    void initWiFi(String ssid, String password) {
      WiFi.mode(WIFI_MODE_AP);
      WiFi.begin(ssid, password);
    }

    void initWiFi(fs::FS &fs,  String filename) {
      initSPIFFS();

      File config = fs.open(filename);

      if(!config) {
        USBSerial.printf("Error on file open: %s\n", filename.c_str());
        return;
      }

      String ssid = config.readStringUntil('\n');
      String pass = config.readStringUntil('\n');
      ssid.trim();
      pass.trim();
      USBSerial.print(ssid);
      USBSerial.print(pass);

      initWiFi(ssid, pass);

      config.close();
    }

    bool WiFiConnected() {
      return WiFi.status() == WL_CONNECTED;
    }

    void saveWiFiCredentials(String ssid, String pass, fs::FS &fs,  String filename) {
      initSPIFFS();

      File config = fs.open(filename, "w");

      if(!config) {
        USBSerial.printf("Error on file open: %s\n", filename.c_str());
        return;
      }

      config.println(ssid);
      config.println(pass);

      config.close();
    }

    void getWiFiCredentials(String* ssid, String* pass, fs::FS &fs,  String filename) {
      initSPIFFS();

      File config = fs.open(filename);

      if(!config) {
        USBSerial.printf("Error on file open: %s\n", filename.c_str());
        return;
      }
      if(ssid != NULL) *ssid = config.readStringUntil('\n');
      if(pass != NULL) *pass = config.readStringUntil('\n');

      config.close();
    }
}