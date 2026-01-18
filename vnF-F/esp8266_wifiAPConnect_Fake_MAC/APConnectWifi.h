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
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <lwip/napt.h>
#include <lwip/dns.h>
#include <LwipDhcpServer.h>
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
    private:
    const String HotSpotName = "AP_THINH_SETUP";
    String WifiName = "VNPT-IT.AP.VNPT-IT.KV5";
    uint8_t newMACAddress[6] = {0xfa, 0x92, 0xbf, 0x04, 0xc3, 0x16};
    const String MAC01 = "fa:92:bf:04:c3:16";
    const String Name01 = "VNPT-IT.AP.VNPT-IT.KV5";
    const String MAC02 = "74:83:c2:91:05:34";
    const String Name02 = "VNPT-IT.AP.Egov-PM4";
    const String MAC03 = "00:1e:78:08:26:3c";
    const String Name03 = "VNPT-IT.AP.P-BanGiamDoc";
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

APConnectWifi::APConnectWifi(int port, int portPortal):server(port),serverPortal(portPortal){
    EEPROM.begin(512); //Initialasing EEPROM
}

void APConnectWifi::begin(void){
}

bool APConnectWifi::checkWifi()
{
    int c = 0;
    while ( c < 20 ) {
        if (WiFi.status() == WL_CONNECTED)
        {
        return true;
        }
        delay(1000);
        Serial.print("*");
        c++;
    }
    Serial.println("");
    Serial.println("Connect timed out...");
    return false;
}  



void APConnectWifi::launchWeb(void)
{
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
  Serial.println();
  Serial.println("Đang chờ bạn thiết lập mạng trên ip:");;
  IPAddress ip = WiFi.softAPIP();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  Serial.print(ipStr);
  while ((WiFi.status() != WL_CONNECTED))
  {
    Serial.print(".");
    delay(1000);
    server.handleClient();
  }
}

void APConnectWifi::launchWebPortal(void)
{
  serverPortal.handleClient();
}


void APConnectWifi::setupAP(void)
{
  Serial.println("Đang mở chế độ kết nối AP");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.softAP(HotSpotName, "");
}

void APConnectWifi::startHostpot(void)
{
  String ssid = ReadSSIDFake();
  String macAddress = ReadMacAddress();
  uint8_t newMACAddressTmp[6]; // Mảng lưu trữ giá trị byte, MAC address có 6 phần tử
  int startPos = 0;
  int endPos = 0;
  for (int i = 0; i < 6; ++i) {
    endPos = macAddress.indexOf(':', startPos); // Tìm vị trí dấu ":" kế tiếp

    if (endPos == -1) {
      endPos = macAddress.length();
    }

    String hexString = macAddress.substring(startPos, endPos); // Tách từng đoạn MAC address
    newMACAddressTmp[i] = strtoul(hexString.c_str(), NULL, 16); // Chuyển đổi chuỗi hex sang giá trị uint8_t
    startPos = endPos + 1;
  }
  if(ssid != ""  &&  ssid[0] != '\0') {
    WifiName = ssid;
  }
  if(macAddress !="" && macAddress[0] != '\0' ){
    for (int i = 0; i < 6; ++i) {
      newMACAddress[i] = newMACAddressTmp[i];
    }
  }
  Serial.printf("setup wifi: %d , mac: %d ", newMACAddress);

  WiFi.setPhyMode(WIFI_PHY_MODE_11N); // Set radio type to N
  WiFi.mode(WIFI_AP_STA);
  WiFi.persistent(false);
  WiFi.begin(); // Use stored credentials to connect to network
  char static_ip[16]="192.168.0.56";
  char static_gw[16]="192.168.0.56";
  char static_sn[16]="255.255.255.0";
  char dns[16]="8.8.8.8";
    IPAddress _ip, _gw, _sn, _dns;
  _ip.fromString(static_ip);
  _gw.fromString(static_gw);
  _sn.fromString(static_sn);
  _dns.fromString(dns);
  WiFi.softAPConfig(  // Set IP Address, Gateway and Subnet
    _ip,
     _gw,
    _sn
    );

  WiFi.softAP(WifiName, "");
  wifi_set_macaddr(SOFTAP_IF, &newMACAddress[0]);
  WiFi.softAPmacAddress(newMACAddress);
Serial.print("[NEW] ESP8266 Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  err_t ret = ip_napt_init(NAPT, NAPT_PORT);
  Serial.printf("ip_napt_init(%d,%d): ret=%d (OK=%d)\n", NAPT, NAPT_PORT, (int)ret, (int)ERR_OK);
  if (ret == ERR_OK) {
    ret = ip_napt_enable_no(SOFTAP_IF, 1);
    Serial.printf("ip_napt_enable_no(SOFTAP_IF): ret=%d (OK=%d)\n", (int)ret, (int)ERR_OK);
    if (ret == ERR_OK) {
      Serial.printf("\nWiFi Network '%s' with Passowrd '%s' and IP '%s' is now setup\n", WiFi.softAPSSID(), WiFi.softAPPSK().c_str(), WiFi.softAPIP().toString().c_str());
    }
  }
  if (ret != ERR_OK) {
    Serial.printf("Lỗi");
  }
}

void APConnectWifi::clearEEPROM(void){
  Serial.println("clearing eeprom");
  for (int i = 0; i < 150; ++i) {
    EEPROM.write(i, 0);
  }
}
void APConnectWifi::writeSSID(String ssid){
  for (int i = 0; i < ssid.length(); ++i)
  {
    EEPROM.write(i, ssid[i]);
    Serial.print(ssid[i]);
  }
}

String APConnectWifi::ReadSSID(){
  String esid;
  for (int i = 0; i < ssidLength; ++i)
  {
    if(EEPROM.read(i) != '\0'){
      esid += char(EEPROM.read(i));
    }
   
  }
  esid.replace("\0", "");
  return esid;
}

String APConnectWifi::ReadPassword(){
  Serial.println("Reading EEPROM pass..");
  String epass = "";
  for (int i = ssidLength; i < passLength; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.println("PASS: ");
  Serial.print(epass);
  return epass;
}

void APConnectWifi::writePassword(String pass){
  Serial.println("writing eeprom pass:");
  for (int i = 0; i < pass.length(); ++i)
  {
    EEPROM.write(ssidLength + i, pass[i]);
    Serial.print(pass[i]);
  }
  Serial.println("");
}

String APConnectWifi::ReadMacAddress(){
  String epass = "";
  for (int i = passLength; i < macLength; ++i)
  {
     if(EEPROM.read(i) != '\0'){
      epass += char(EEPROM.read(i));
     }
  }
  epass.replace("\0", "");
  return epass;
}

void APConnectWifi::writeMacAddress(String pass){
  for (int i = 0; i < pass.length(); ++i)
  {
    EEPROM.write(passLength + i, pass[i]);
  }
}

void APConnectWifi::writeSSIDFake(String ssid){
  for (int i = 0; i < ssid.length(); ++i)
  {
   EEPROM.write(macLength + i, ssid[i]);
  }
}

String APConnectWifi::ReadSSIDFake(){
  String esid;
  for (int i = macLength; i < ssidFakeLength; ++i)
  {
    if(EEPROM.read(i) != '\0'){
      esid += char(EEPROM.read(i));
    }
   
  }
  esid.replace("\0", "");
  return esid;
}

void APConnectWifi::clearSSIDFake(void){
  Serial.println("clearing eeprom");
  for (int i = macLength; i < ssidFakeLength; ++i) {
    EEPROM.write(i, 0);
  }
}

void APConnectWifi::createWebServer()
{

    server.on("/", [this]() {

      IPAddress ip = WiFi.softAPIP();
      int n = WiFi.scanNetworks();
      st = "<select  onchange='c(this)'>";
      st+= "<option>select wifi</option>";
      for (int i = 0; i < n; ++i)
      {
        // Print SSID and RSSI for each network found
        st += "<option value='"+ WiFi.SSID(i) +"'>";
        st += WiFi.SSID(i);
        st += " (";
        st += WiFi.RSSI(i);

        st += ")";
        st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
        st += "</option>";
      }
      st += "</select>";
      st += "<br>";
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE html><html lang=\"en\">";
      content +="<head>";
      content +="<meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
      content += "<title>   Welcome to Wifi Credentials Update pag</title>";
      content += "<style>input,select{display: block; margin-bottom: 1em; padding: 0.5em} body{font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;}</style>";
      content += "<script>function c(l){document.getElementById('name').value=l.value;document.getElementById('p').focus();}</script>";
      content += "</head>";
      content += "<body>";
      content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
      content += ipStr;
      content += st;
      content += "<form method='get' action='setting'><label>SSID: </label><input placeholder='name wifi' name='ssid' id='name' length=32><input id='p' name='pass' placeholder='password' length=64><input type='submit'></form>";
      content += "</body>";
      content += "</html>";
      server.send(200, "text/html", content);
    });
    server.on("/scan", [this]() {
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
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
        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
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


void APConnectWifi::startWebPortal(){
    serverPortal.on("/", [this]() {
      IPAddress ip = WiFi.localIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      String ssid = this->ReadSSID();
      String macAddress = this->ReadMacAddress();
      st = "<select onchange='c(this)'>";
      st += "<option>select wifi</option>";
      st += "<option value='"+ MAC01 + "|"+ Name01 +"'>"+ Name01 + "</option>";
      st += "<option value='"+ MAC02 + "|"+ Name02 +"'>"+ Name02 + "</option>";
      st += "<option value='"+ MAC03 + "|"+ Name03 +"'>"+ Name03 + "</option>";
      st += "</select>";
      st += "<br>";
      content = "<!DOCTYPE html><html lang=\"en\">";
      content +="<head>";
      content +="<meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
      content += "<title>   Welcome to Wifi Credentials Update pag</title>";
      content += "<style>input,select{display: block; margin-bottom: 1em; padding: 0.5em} body{font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;}</style>";
      content += "<script>function c(l){ var arr=l.value.split('|');document.getElementById('name').value=arr[1];document.getElementById('mac').value=arr[0]; document.getElementById('mac').focus();}</script>";
      content += "</head>";
      content += "<body>";
      content += ipStr;
      content += st;
      content += "<form method='get' action='setting'>";
      content += "<label> Tên wifi FAKE </label>";
      content += "<input placeholder='name wifi' name='ssid' id='name' length=32 value='"+ssid+"'>";
      content += "<label> Địa chỉ Mac FAKE </label>";
      content += "<input id='mac' name='mac' placeholder='Mac address' length=64 value='"+macAddress+"'>";
      content += "<p> Vd: 00:1e:78:08:26:3c</p>";
      content += "<input type='submit'></form>";
      content += "</body>";
      content += "</html>";
      serverPortal.send(200, "text/html", content);
    });
    serverPortal.on("/setting", [this]() {
      String qsid = serverPortal.arg("ssid");
      String qmacAddress = serverPortal.arg("mac");
      if (qsid.length() > 0 && qmacAddress.length() > 0) {
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qmacAddress);
        Serial.println("");
        this->clearSSIDFake();
        this->writeSSIDFake(qsid);
        this->writeMacAddress(qmacAddress);
        EEPROM.commit();
        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        serverPortal.sendHeader("Location", "/"); // Điều hướng đến "/new_page"
        serverPortal.send(302, "text/plain", "");
        delay(1000);
        ESP.reset();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      serverPortal.sendHeader("Access-Control-Allow-Origin", "*");
      serverPortal.send(statusCode, "application/json", content);
    });
    serverPortal.begin();
}
 
#endif
