#define BLYNK_PRINT Serial
#include "APConnectWifi.h"
#include <ESP8266WiFi.h>
#include <Wire.h>

String ssid = "ThinhHuynh";
String password = "88888888";
// int ledPin = D3;
// int button = D7;
//  Thiết lập port webserver khi kết nối wifi
APConnectWifi APWifi(8081, 80);
void setup() {
  Serial.begin(115200);
  Serial.println("Disconnecting current wifi connection");
  WiFi.disconnect();
  delay(10);
  APWifi.setLedPin(2); // Enable LED status on GPIO2 (Built-in LED)
  Serial.println("Startup wifi");
  //---------------------------------------- Read eeprom for ssid and pass
  String esid = APWifi.ReadSSID();
  String epass = APWifi.ReadPassword();
  WiFi.begin(esid.c_str(), epass.c_str());
  if (!APWifi.checkWifi()) {
    Serial.println("Bắt đầu mở HotSpot, vui lòng kết nối wifi");
    APWifi.setupAP();
    APWifi.launchWeb();
    return;
  }
  APWifi.startHostpot();
  Serial.println("Succesfully Connected!!!");
  IPAddress ip = WiFi.localIP();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) +
                 '.' + String(ip[3]);
  Serial.println(ipStr);
  APWifi.startWebPortal();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    APWifi.launchWebPortal();
  }
}
