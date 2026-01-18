/**
 * @file       APConnectWifi.h
 * @author     Tuancm
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2023 Tuancm
 * @date       Jan 2023
 * @brief
 *
 */
#ifndef APConnectWifi_h
#define APConnectWifi_h
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

extern "C" {
#include <user_interface.h>
}
#include <LwipDhcpServer.h>
#include <lwip/dns.h>
#include <lwip/napt.h>

#define NAPT 1000
#define NAPT_PORT 10
class APConnectWifi {
public:
  APConnectWifi(int, int);
  void begin(void);
  bool checkWifi(void);
  void launchWeb(void);
  void setupAP(void);
  void startHostpot(void);
  void clearEEPROM(void);
  void writeSSID(String);
  String ReadPassword(void);
  void writePassword(String);
  String ReadSSID(void);
  void writeSSIDFake(String);
  String ReadSSIDFake(void);
  void clearSSIDFake(void);
  String ReadMacAddress(void);
  void writeMacAddress(String);
  void createWebServer(void);
  void startWebPortal(void);
  void launchWebPortal(void);
  void setLedPin(int pin);

private:
  void handleDashboard();
  void handleWifiSetup();
  void handleApSetup();
  void handleScan();
  void handleSaveWifi();
  void handleSaveAp();
  String getCommonStyle();
  String getHeader(String title);
  String getFooter();
  int _ledPin = -1;
  const String HotSpotName = "AP_THINH_SETUP";
  String WifiName = "VNPT-ICS";
  uint8_t newMACAddress[6] = {0xfe, 0x00, 0x02, 0x00, 0x2c, 0xc0};
  const String MAC01 = "8a:78:ba:d2:65:23";
  const String Name01 = "VNPT-ICS";
  const String MAC02 = "74:83:c2:91:05:34";
  const String Name02 = "VNPT-IT.AP.Egov-PM4";
  const String MAC03 = "00:1e:78:08:26:3c";
  const String Name03 = "VNPT-IT.AP.P-BanGiamDoc";
  const String MAC04 = "92:36:7d:01:22:66";
  const String Name04 = "VNPT-IT.AP.VNPT-IT.KV5-NEW";
  const int ssidLength = 32;
  const int passLength = 15 + ssidLength;
  const int macLength = 17 + passLength;
  const int ssidFakeLength = 32 + macLength;
  String content;
  int statusCode;
  String st;
  ESP8266WebServer server;
  ESP8266WebServer serverPortal;
};

APConnectWifi::APConnectWifi(int port, int portPortal)
    : server(port), serverPortal(portPortal) {
  EEPROM.begin(512); // Initialasing EEPROM
}

void APConnectWifi::begin(void) {}

void APConnectWifi::setLedPin(int pin) {
  _ledPin = pin;
  pinMode(_ledPin, OUTPUT);
  digitalWrite(_ledPin, HIGH); // Off by default (assuming active LOW)
}

bool APConnectWifi::checkWifi() {
  int c = 0;
  while (c < 40) { // Increased timeout slightly for better visibility
    if (WiFi.status() == WL_CONNECTED) {
      if (_ledPin != -1)
        digitalWrite(_ledPin, LOW); // ON (Active LOW)
      return true;
    }
    if (_ledPin != -1) {
      digitalWrite(_ledPin, !digitalRead(_ledPin)); // Blink
    }
    delay(500); // Faster blink
    Serial.print("*");
    c++;
  }
  if (_ledPin != -1)
    digitalWrite(_ledPin, HIGH); // OFF
  Serial.println("");
  Serial.println("Connect timed out...");
  return false;
}

void APConnectWifi::launchWeb(void) {
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
  Serial.println();
  Serial.println("Đang chờ bạn thiết lập mạng trên ip:");
  ;
  IPAddress ip = WiFi.softAPIP();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) +
                 '.' + String(ip[3]);
  Serial.print(ipStr);
  while ((WiFi.status() != WL_CONNECTED)) {
    Serial.print(".");
    delay(1000);
    server.handleClient();
  }
}

void APConnectWifi::launchWebPortal(void) { serverPortal.handleClient(); }

void APConnectWifi::setupAP(void) {
  Serial.println("Đang mở chế độ kết nối AP");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.softAP(HotSpotName, "");
}

void APConnectWifi::startHostpot(void) {
  String ssid = ReadSSIDFake();
  String macAddress = ReadMacAddress();
  uint8_t newMACAddressTmp[6]; // Mảng lưu trữ giá trị byte, MAC address có 6
                               // phần tử
  int startPos = 0;
  int endPos = 0;
  for (int i = 0; i < 6; ++i) {
    endPos = macAddress.indexOf(':', startPos); // Tìm vị trí dấu ":" kế tiếp

    if (endPos == -1) {
      endPos = macAddress.length();
    }

    String hexString =
        macAddress.substring(startPos, endPos); // Tách từng đoạn MAC address
    newMACAddressTmp[i] =
        strtoul(hexString.c_str(), NULL,
                16); // Chuyển đổi chuỗi hex sang giá trị uint8_t
    startPos = endPos + 1;
  }
  if (ssid != "" && ssid[0] != '\0') {
    WifiName = ssid;
  }
  if (macAddress != "" && macAddress[0] != '\0') {
    for (int i = 0; i < 6; ++i) {
      newMACAddress[i] = newMACAddressTmp[i];
    }
  }
  Serial.printf("setup wifi: %d , mac: %d ", newMACAddress);

  WiFi.setPhyMode(WIFI_PHY_MODE_11N); // Set radio type to N
  WiFi.mode(WIFI_AP_STA);
  WiFi.mode(WIFI_AP_STA);
  // WiFi.persistent(false); // Do not change persistence mid-flight if not
  // needed WiFi.begin(); // Removed: This was causing
  // reconnection/disconnection
  char static_ip[16] = "192.168.0.56";
  char static_gw[16] = "192.168.0.56";
  char static_sn[16] = "255.255.255.0";
  char dns[16] = "8.8.8.8";
  IPAddress _ip, _gw, _sn, _dns;
  _ip.fromString(static_ip);
  _gw.fromString(static_gw);
  _sn.fromString(static_sn);
  _dns.fromString(dns);
  WiFi.softAPConfig( // Set IP Address, Gateway and Subnet
      _ip, _gw, _sn);

  // Configure DHCP DNS - REMOVED due to linker error, relying on default
  // behavior + NAPT wifi_softap_dhcps_stop(); struct ip_addr dns_addr;
  // IP4_ADDR(&dns_addr, 8, 8, 8, 8);
  // dhcps_set_DNS(&dns_addr);
  // wifi_softap_dhcps_start();

  WiFi.softAP(WifiName, "");
  wifi_set_macaddr(SOFTAP_IF, &newMACAddress[0]);
  WiFi.softAPmacAddress(newMACAddress);
  Serial.print("[NEW] ESP8266 Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  err_t ret = ip_napt_init(NAPT, NAPT_PORT);
  Serial.printf("ip_napt_init(%d,%d): ret=%d (OK=%d)\n", NAPT, NAPT_PORT,
                (int)ret, (int)ERR_OK);
  if (ret == ERR_OK) {
    ret = ip_napt_enable_no(SOFTAP_IF, 1);
    Serial.printf("ip_napt_enable_no(SOFTAP_IF): ret=%d (OK=%d)\n", (int)ret,
                  (int)ERR_OK);
    if (ret == ERR_OK) {
      Serial.printf(
          "\nWiFi Network '%s' with Passowrd '%s' and IP '%s' is now setup\n",
          WiFi.softAPSSID(), WiFi.softAPPSK().c_str(),
          WiFi.softAPIP().toString().c_str());
    }
  }
  if (ret != ERR_OK) {
    Serial.printf("Lỗi");
  }
}

void APConnectWifi::clearEEPROM(void) {
  Serial.println("clearing eeprom");
  for (int i = 0; i < 150; ++i) {
    EEPROM.write(i, 0);
  }
}
void APConnectWifi::writeSSID(String ssid) {
  for (int i = 0; i < ssid.length(); ++i) {
    EEPROM.write(i, ssid[i]);
    Serial.print(ssid[i]);
  }
}

String APConnectWifi::ReadSSID() {
  String esid;
  for (int i = 0; i < ssidLength; ++i) {
    if (EEPROM.read(i) != '\0') {
      esid += char(EEPROM.read(i));
    }
  }
  esid.replace("\0", "");
  return esid;
}

String APConnectWifi::ReadPassword() {
  Serial.println("Reading EEPROM pass..");
  String epass = "";
  for (int i = ssidLength; i < passLength; ++i) {
    epass += char(EEPROM.read(i));
  }
  Serial.println("PASS: ");
  Serial.print(epass);
  return epass;
}

void APConnectWifi::writePassword(String pass) {
  Serial.println("writing eeprom pass:");
  for (int i = 0; i < pass.length(); ++i) {
    EEPROM.write(ssidLength + i, pass[i]);
    Serial.print(pass[i]);
  }
  Serial.println("");
}

String APConnectWifi::ReadMacAddress() {
  String epass = "";
  for (int i = passLength; i < macLength; ++i) {
    if (EEPROM.read(i) != '\0') {
      epass += char(EEPROM.read(i));
    }
  }
  epass.replace("\0", "");
  return epass;
}

void APConnectWifi::writeMacAddress(String pass) {
  for (int i = 0; i < pass.length(); ++i) {
    EEPROM.write(passLength + i, pass[i]);
  }
}

void APConnectWifi::writeSSIDFake(String ssid) {
  for (int i = 0; i < ssid.length(); ++i) {
    EEPROM.write(macLength + i, ssid[i]);
  }
}

String APConnectWifi::ReadSSIDFake() {
  String esid;
  for (int i = macLength; i < ssidFakeLength; ++i) {
    if (EEPROM.read(i) != '\0') {
      esid += char(EEPROM.read(i));
    }
  }
  esid.replace("\0", "");
  return esid;
}

void APConnectWifi::clearSSIDFake(void) {
  Serial.println("clearing eeprom");
  for (int i = macLength; i < ssidFakeLength; ++i) {
    EEPROM.write(i, 0);
  }
}

void APConnectWifi::createWebServer() {

  server.on("/", [this]() {
    IPAddress ip = WiFi.softAPIP();
    int n = WiFi.scanNetworks();
    st = "<select  onchange='c(this)'>";
    st += "<option>select wifi</option>";
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      st += "<option value='" + WiFi.SSID(i) + "'>";
      st += WiFi.SSID(i);
      st += " (";
      st += WiFi.RSSI(i);

      st += ")";
      st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
      st += "</option>";
    }
    st += "</select>";
    st += "<br>";
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) +
                   '.' + String(ip[3]);
    content = "<!DOCTYPE html><html lang=\"en\">";
    content += "<head>";
    content +=
        "<meta charset=\"UTF-8\"><meta name=\"viewport\" "
        "content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
    content += "<title>   Welcome to Wifi Credentials Update pag</title>";
    content += "<style>input,select{display: block; margin-bottom: 1em; "
               "padding: 0.5em} body{font-family:verdana;} "
               "button{border:0;border-radius:0.3rem;background-color:#1fa3ec;"
               "color:#fff;line-height:2.4rem;font-size:1.2rem;}</style>";
    content += "<script>function "
               "c(l){document.getElementById('name').value=l.value;document."
               "getElementById('p').focus();}</script>";
    content += "</head>";
    content += "<body>";
    content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" "
               "value=\"scan\"></form>";
    content += ipStr;
    content += st;
    content += "<form method='get' action='setting'><label>SSID: "
               "</label><input placeholder='name wifi' name='ssid' id='name' "
               "length=32><input id='p' name='pass' placeholder='password' "
               "length=64><input type='submit'></form>";
    content += "</body>";
    content += "</html>";
    server.send(200, "text/html", content);
  });
  server.on("/scan", [this]() {
    IPAddress ip = WiFi.softAPIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) +
                   '.' + String(ip[3]);
    content = "<!DOCTYPE HTML>\r\n<html>go back";
    server.send(200, "text/html", content);
  });
  server.on("/setting", [this]() {
    String qsid = server.arg("ssid");
    String qpass = server.arg("pass");
    if (qsid.length() > 0 && qpass.length() > 0) {
      this->clearEEPROM();
      Serial.println(qsid);
      Serial.println("");
      Serial.println(qpass);
      Serial.println("");
      this->writeSSID(qsid);
      this->writePassword(qpass);
      EEPROM.commit();
      content =
          "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
      statusCode = 200;
      ESP.reset();
    } else {
      content = "{\"Error\":\"404 not found\"}";
      statusCode = 404;
      Serial.println("Sending 404");
    }
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(statusCode, "application/json", content);
  });
}

void APConnectWifi::startWebPortal() {
  serverPortal.on("/", [this]() { this->handleDashboard(); });
  serverPortal.on("/wifi-setup", [this]() { this->handleWifiSetup(); });
  serverPortal.on("/ap-setup", [this]() { this->handleApSetup(); });
  serverPortal.on("/scan", [this]() { this->handleScan(); });
  serverPortal.on("/save-wifi", [this]() { this->handleSaveWifi(); });
  serverPortal.on("/save-ap", [this]() { this->handleSaveAp(); });

  // Keep legacy references or redirect?
  // Let's keep specific setting endpoints if needed or just use new ones

  serverPortal.begin();
}

String APConnectWifi::getCommonStyle() {
  return "<style>"
         "body{font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; "
         "margin: 0; padding: 0; background-color: #f4f4f9; color: #333;}"
         ".container{max-width: 600px; margin: 20px auto; padding: 20px; "
         "background: #fff; border-radius: 8px; box-shadow: 0 2px 10px "
         "rgba(0,0,0,0.1);}"
         "h1{color: #007bff; text-align: center;}"
         "h2{border-bottom: 2px solid #007bff; padding-bottom: 10px; "
         "margin-top: 20px;}"
         ".card{background: #e9ecef; padding: 15px; border-radius: 5px; "
         "margin-bottom: 15px;}"
         ".btn{display: inline-block; padding: 10px 20px; color: #fff; "
         "background-color: #007bff; border: none; border-radius: 5px; "
         "text-decoration: none; text-align: center; cursor: pointer; width: "
         "100%; box-sizing: border-box; margin-top: 5px;}"
         ".btn:hover{background-color: #0056b3;}"
         ".btn-secondary{background-color: #6c757d;}"
         ".btn-secondary:hover{background-color: #545b62;}"
         "input, select{width: 100%; padding: 10px; margin: 5px 0 15px; "
         "border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box;}"
         "label{font-weight: bold;}"
         ".status-ok{color: green; font-weight: bold;}"
         ".status-err{color: red; font-weight: bold;}"
         "</style>";
}

String APConnectWifi::getHeader(String title) {
  String h = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">";
  h += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, "
       "user-scalable=no\"/>";
  h += "<title>" + title + "</title>";
  h += getCommonStyle();
  h += "</head><body><div class=\"container\">";
  h += "<h1>" + title + "</h1>";
  return h;
}

String APConnectWifi::getFooter() {
  return "<div style='text-align:center; margin-top:20px; font-size:0.8em; "
         "color:#777;'>APConnectWifi System</div></div></body></html>";
}

void APConnectWifi::handleDashboard() {
  String p = getHeader("System Dashboard");

  // Status Section
  p += "<div class=\"card\"><h2>Connection Status</h2>";
  if (WiFi.status() == WL_CONNECTED) {
    p += "<p>Status: <span class=\"status-ok\">Connected</span></p>";
    p += "<p>Connected to: <strong>" + WiFi.SSID() + "</strong></p>";
    p +=
        "<p>IP Address: <strong>" + WiFi.localIP().toString() + "</strong></p>";
    p += "<p>Signal Strength: <strong>" + String(WiFi.RSSI()) +
         " dBm</strong></p>";
  } else {
    p += "<p>Status: <span class=\"status-err\">Disconnected</span></p>";
  }
  p += "</div>";

  // AP Status Section
  p += "<div class=\"card\"><h2>Access Point</h2>";
  p += "<p>SSID: <strong>" + String(WiFi.softAPSSID()) + "</strong></p>";
  p += "<p>IP: <strong>" + WiFi.softAPIP().toString() + "</strong></p>";
  p += "<p>MAC: <strong>" + WiFi.softAPmacAddress() + "</strong></p>";
  p += "</div>";

  // Menu Links
  p += "<a href=\"/wifi-setup\" class=\"btn\">Configure Source WiFi</a>";
  p +=
      "<a href=\"/ap-setup\" class=\"btn btn-secondary\">Configure Fake AP</a>";

  p += getFooter();
  serverPortal.send(200, "text/html", p);
}

void APConnectWifi::handleWifiSetup() {
  // Logic from createWebServer / setting
  String p = getHeader("Configure Source WiFi");

  p += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" "
       "value=\"Scan Networks\" class=\"btn\"></form>";

  p += "<br><form method='POST' action='/save-wifi'>";
  p += "<label>SSID</label><input placeholder='WiFi Name' name='ssid' "
       "id='ssid'>";
  p += "<label>Password</label><input type='password' name='pass' "
       "placeholder='Password'>";
  p += "<input type='submit' value='Connect' class=\"btn\">";
  p += "</form>";

  p += "<br><a href=\"/\" class=\"btn btn-secondary\">Back to Dashboard</a>";
  p += getFooter();
  serverPortal.send(200, "text/html", p);
}

void APConnectWifi::handleScan() {
  String p = getHeader("Select Network");
  int n = WiFi.scanNetworks();

  if (n == 0) {
    p += "<p>No networks found.</p>";
  } else {
    p += "<script>function "
         "c(s){document.getElementById('ssid').value=s;}</script>";
    p += "<div style='max-height: 300px; overflow-y: auto;'>";
    for (int i = 0; i < n; ++i) {
      p += "<div class='card' onclick=\"c('" + WiFi.SSID(i) +
           "')\" style='cursor:pointer;'>";
      p += "<strong>" + WiFi.SSID(i) + "</strong>";
      p += " (" + String(WiFi.RSSI(i)) + " dBm)";
      p += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " [Open]" : " [Secured]";
      p += "</div>";
    }
    p += "</div>";
  }

  p += "<form method='POST' action='/save-wifi'>";
  p += "<label>Selected SSID</label><input placeholder='WiFi Name' name='ssid' "
       "id='ssid'>";
  p += "<label>Password</label><input type='password' name='pass' "
       "placeholder='Password'>";
  p += "<input type='submit' value='Connect' class=\"btn\">";
  p += "</form>";

  p += "<br><a href=\"/wifi-setup\" class=\"btn btn-secondary\">Back</a>";
  p += getFooter();
  serverPortal.send(200, "text/html", p);
}

void APConnectWifi::handleSaveWifi() {
  String qsid = serverPortal.arg("ssid");
  String qpass = serverPortal.arg("pass");
  if (qsid.length() > 0 && qpass.length() > 0) { // Allow empty pass? maybe
    this->clearEEPROM();
    this->writeSSID(qsid);
    this->writePassword(qpass);
    EEPROM.commit();

    String p = getHeader("Configuration Saved");
    p += "<div class='card'><p class='status-ok'>WiFi credentials "
         "saved!</p><p>The device will now restart to connect.</p></div>";
    p += getFooter();
    serverPortal.send(200, "text/html", p);
    delay(2000);
    ESP.reset();
  } else {
    String p = getHeader("Error");
    p += "<div class='card'><p class='status-err'>SSID cannot be "
         "empty.</p></div>";
    p += "<a href=\"/wifi-setup\" class=\"btn\">Try Again</a>";
    p += getFooter();
    serverPortal.send(200, "text/html", p);
  }
}

void APConnectWifi::handleApSetup() {
  String p = getHeader("Configure Fake AP");
  String ssid = this->ReadSSIDFake();
  String macAddress = this->ReadMacAddress();

  p += "<form method='POST' action='/save-ap'>";
  p += "<label>Fake AP Name (SSID)</label>";
  p += "<input placeholder='AP Name' name='ssid' value='" + ssid + "'>";

  p += "<label>Fake MAC Address</label>";
  p += "<input placeholder='XX:XX:XX:XX:XX:XX' name='mac' value='" +
       macAddress + "'>";

  p += "<div class='card'><p>Presets:</p>";
  p += "<button type='button' class='btn btn-secondary' onclick=\"s('" + MAC01 +
       "','" + Name01 + "')\">" + Name01 + "</button>";
  p += "<button type='button' class='btn btn-secondary' onclick=\"s('" + MAC02 +
       "','" + Name02 + "')\">" + Name02 + "</button>";
  p += "<button type='button' class='btn btn-secondary' onclick=\"s('" + MAC03 +
       "','" + Name03 + "')\">" + Name03 + "</button>";
  p += "<button type='button' class='btn btn-secondary' onclick=\"s('" + MAC04 +
       "','" + Name04 + "')\">" + Name04 + "</button>";
  p += "</div>";

  p += "<script>function "
       "s(m,n){document.getElementsByName('ssid')[0].value=n;document."
       "getElementsByName('mac')[0].value=m;}</script>";

  p += "<input type='submit' value='Save & Restart' class=\"btn\">";
  p += "</form>";

  p += "<br><a href=\"/\" class=\"btn btn-secondary\">Back to Dashboard</a>";
  p += getFooter();
  serverPortal.send(200, "text/html", p);
}

void APConnectWifi::handleSaveAp() {
  String qsid = serverPortal.arg("ssid");
  String qmacAddress = serverPortal.arg("mac");
  if (qsid.length() > 0 && qmacAddress.length() > 0) {
    this->clearSSIDFake();
    this->writeSSIDFake(qsid);
    this->writeMacAddress(qmacAddress);
    EEPROM.commit();

    String p = getHeader("Configuration Saved");
    p += "<div class='card'><p class='status-ok'>AP Settings "
         "saved!</p><p>Device restarting...</p></div>";
    p += getFooter();
    serverPortal.send(200, "text/html", p);
    delay(2000);
    ESP.reset();
  } else {
    String p = getHeader("Error");
    p += "<div class='card'><p class='status-err'>Invalid Input.</p></div>";
    p += "<a href=\"/ap-setup\" class=\"btn\">Try Again</a>";
    p += getFooter();
    serverPortal.send(200, "text/html", p);
  }
}

#endif
