/*
 * misk.cpp
 *
 *  Created on: 22 ����. 2019 �.
 *      Author: ominenko
 */
#include "misk.h"
#include "logs.h"
#include <FS.h>
#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#include <ArduinoJson.h>
#include "fatlib\ArduinoStream.h"
#include <stdio.h>
#include <sstream>
using namespace sdfat;

ostream& operator<<(ostream &stream, const String &str)
{
    stream << str.c_str();
    return stream;
}

ostream& operator<<(ostream &stream, const IPAddress &ip)
{
    stream << ip.toString();
    return stream;
}

void LED_ON()
{
    digitalWrite(LED_BUILTIN, LOW);
}
void LED_OFF()
{
    digitalWrite(LED_BUILTIN, HIGH);
}
void blink()
{
    LED_ON();
    delay(100);
    LED_OFF();
    delay(400);
}

bool setup_wifi(const String &ssid, const String &pwd, const String &host_name, const WiFiMode_t &wifi_mode)
{
    // Set hostname first
    DBG_PRINT(F("hostname:"));
    DBG_PRINTLN(host_name);
    DBG_PRINT(F("pwd: "));
    DBG_PRINTLN(pwd);
    DBG_PRINT(F("Mode: "));
    DBG_PRINTLN(WiFi.getPhyMode())
    WiFi.hostname(host_name);
    // Reduce startup surge current
    WiFi.setAutoConnect(false);
    WiFi.mode(wifi_mode);
    WiFi.setPhyMode(WIFI_PHY_MODE_11N);
    if (WIFI_STA == wifi_mode)
    {
        WiFi.begin(ssid.c_str(), pwd.c_str());
        Serial.print("M117 conn..");
        Serial.println(ssid);
        // Wait for connection
        while (WiFi.status() != WL_CONNECTED)
        {
            blink();
            DBG_PRINT(".");
        }
        DBG_PRINT(F( "IP address:"));
        DBG_PRINTLN(WiFi.localIP().toString());
    } else
    {
        WiFi.mode(WIFI_AP);
        WiFi.softAP(host_name.c_str(), pwd.c_str());
        DBG_PRINT(F( "AP IP address:"));
        DBG_PRINTLN(WiFi.softAPIP().toString());
    }
    DBG_PRINT(F("RSSI: "));
    DBG_PRINTLN(WiFi.RSSI());
    return true;
}

bool get_config(Stream &config, String &ssid, String &pwd, String &name, WiFiMode_t &WiFiMode)
{
    StaticJsonDocument < 512 > doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, config);
    if (error)
    {
        ERR_LOG(F("Failed to read file, using default configuration"));
        return false;
    }
    // Get the root object in the document
    JsonObject root = doc.as<JsonObject>();
    if (!root.containsKey("ssid")
            || !root.containsKey("pwd")
            || !root.containsKey("hostname")
            || !root.containsKey("mode"))
    {
        ERR_LOG(F("with fields { 'ssid':...,'pwd':...,'hostname':...,'mode':...}"));
        return false;
    }
    ssid = root["ssid"].as<String>();
    pwd = root["pwd"].as<String>();
    name = root["hostname"].as<String>();
    WiFiMode = static_cast<WiFiMode_t>(root["mode"].as<int8_t>());

    DBG_PRINT(F("ssid: "));
    DBG_PRINTLN(ssid);
    DBG_PRINT(F("pwd: " ));
    DBG_PRINTLN(pwd);
    DBG_PRINT(F( "hostname: "));
    DBG_PRINTLN(name);
    DBG_PRINT(F( "mode: "));
    DBG_PRINTLN(WiFiMode);
    return true;
}

String getMimeType(String path)
{
// ------------------------
    if (path.endsWith(".html"))
        return "text/html";
    else if (path.endsWith(".htm"))
        return "text/html";
    else if (path.endsWith(".css"))
        return "text/css";
    else if (path.endsWith(".txt"))
        return "text/plain";
    else if (path.endsWith(".js"))
        return "application/javascript";
    else if (path.endsWith(".json"))
        return "application/json";
    else if (path.endsWith(".png"))
        return "image/png";
    else if (path.endsWith(".gif"))
        return "image/gif";
    else if (path.endsWith(".jpg"))
        return "image/jpeg";
    else if (path.endsWith(".ico"))
        return "image/x-icon";
    else if (path.endsWith(".svg"))
        return "image/svg+xml";
    else if (path.endsWith(".ttf"))
        return "application/x-font-ttf";
    else if (path.endsWith(".otf"))
        return "application/x-font-opentype";
    else if (path.endsWith(".woff"))
        return "application/font-woff";
    else if (path.endsWith(".woff2"))
        return "application/font-woff2";
    else if (path.endsWith(".eot"))
        return "application/vnd.ms-fontobject";
    else if (path.endsWith(".sfnt"))
        return "application/font-sfnt";
    else if (path.endsWith(".xml"))
        return "text/xml";
    else if (path.endsWith(".pdf"))
        return "application/pdf";
    else if (path.endsWith(".zip"))
        return "application/zip";
    else if (path.endsWith(".gz"))
        return "application/x-gzip";
    else if (path.endsWith(".appcache"))
        return "text/cache-manifest";
    return "application/octet-stream";
}

void hw_info(Stream &Out)
{
    Out.println("hw_info");
    Out.printf("CpuFreqMHz %u \n", ESP.getCpuFreqMHz());
    Out.printf("getFreeHeap %u \n", ESP.getFreeHeap());
    auto realSize = ESP.getFlashChipRealSize();
    auto ideSize = ESP.getFlashChipSize();
    auto ideMode = ESP.getFlashChipMode();
    Out.println("Flash info");
    Out.printf("id:   %08X\n", ESP.getFlashChipId());
    Out.printf("size: %u\n", realSize);
    if (ideSize != realSize)
    {
        Out.printf("\n!!Different size\nFlash IDE size: %u\n\n", ideSize);
    }
    Out.printf("ide speed: %u\n", ESP.getFlashChipSpeed());
    Out.print("ide mode: ");
    switch (ideMode)
    {
        case FM_QIO:
            Out.println("QIO");
            break;
        case FM_QOUT:
            Out.println("QOUT");
            break;
        case FM_DIO:
            Out.println("DIO");
            break;
        case FM_DOUT:
            Out.println("DOUT");
            break;
        default:
            Out.print(ideMode);
            Out.println("UNKNOWN");
    }
    Out.println("end Flash info");
}

void SPIFFS_info(Stream &out_stream)
{
    ArduinoOutStream cout(out_stream);
    cout << "SPIFFS_info" << endl;
    FSInfo info;
    SPIFFS.info(info);

    cout << "Total:" << info.totalBytes << endl;
    cout << "Used:" << info.usedBytes << endl;
    cout << "nBlock:" << info.blockSize << " Page:" << info.pageSize << endl;
    cout << "nMax open files:" << info.maxOpenFiles << endl;
    cout << "maxPathLength:" << info.maxPathLength << endl;

    cout << "SPIFFS files:" << endl;
    auto dir = SPIFFS.openDir("/");
    while (dir.next())
    {
        cout << dir.fileName() << " SZ:" << dir.fileSize() << endl;
    }
}

void webRetResult(ESP8266WebServer &server, te_ret res)
{
    DBG_PRINT("Err ");
    DBG_PRINTLN(res);
    switch (res) {
        case er_ok:
            server.send(200, "text/plain", "ok");
            break;
        case er_fileNotFound:
            server.send(404, "text/plain", "FileNotFound");
            break;
        case er_openFile:
            server.send(404, "text/plain", "er_openFile");
            break;
        case er_createFile:
            server.send(500, "text/plain", "er_createFile");
            break;
        case er_incorrectMode:
            server.send(400, "text/plain", "er_incorrectMode");
            break;
        case er_no_parameters:
            server.send(400, "text/plain", "er_no_parameters");
            break;
        case er_errorResult:
            server.send(401, "text/plain", "er_errorResult");
            break;
        case er_BuffOverflow:
            server.send(400, "text/plain", "er_BuffOverflow");
            break;
        case err_MarlinRead:
            server.send(400, "text/plain", "err_MarlinRead");
            break;
        case er_FileIO:
            server.send(400, "text/plain", "er_FileIO");
            break;
        default:
            server.send(400, "text/plain", "undefined");
            break;
    }
}

bool isExtMach(const std::string &name, const std::string &ext)
{
    const auto pos = name.find_last_of('.');
    if (std::string::npos == pos)
    {
        return false; //no extention
    }
    const auto fext = name.substr(pos + 1);
    if (fext.length() != ext.length())
    {
        return false;
    }
    auto n = fext.length();
    while (n--)
    {
        if (std::toupper(fext.at(n)) != std::toupper(ext.at(n)))
        {
            return false;
        }
    }
    return true;
}
std::string getResetInfo() {
    std::ostringstream info;
    const auto rst_info = system_get_rst_info();
    info << "rst_info " << rst_info->reason << ":";
    switch (rst_info->reason) {
        case REASON_DEFAULT_RST:
            info << "REASON_DEFAULT_RST";
            break;
        case REASON_WDT_RST: /* hardware watch dog reset */
            info << "REASON_WDT_RST";
            break;
        case REASON_EXCEPTION_RST: /* exception reset, GPIO status won’t change */
            info << "REASON_EXCEPTION_RST";
            break;
        case REASON_SOFT_WDT_RST: /* software watch dog reset, GPIO status won’t change */
            info << "REASON_SOFT_WDT_RST";
            break;
        case REASON_SOFT_RESTART: /* software restart ,system_restart , GPIO status won’t change */
            info << "REASON_SOFT_RESTART";
            break;
        case REASON_DEEP_SLEEP_AWAKE: /* wake up from deep-sleep */
            info << "REASON_DEEP_SLEEP_AWAKE";
            break;
        case REASON_EXT_SYS_RST:/* external system reset */
            info << "REASON_EXT_SYS_RST";
            break;
        default:
            info << "unknow";
            break;
    }
    if (rst_info->reason == REASON_WDT_RST ||
            rst_info->reason == REASON_EXCEPTION_RST ||
            rst_info->reason == REASON_SOFT_WDT_RST) {
        if (rst_info->reason == REASON_EXCEPTION_RST) {
            info << " exccause " << rst_info->exccause;

        }

        info << " epc1=" << std::hex << rst_info->epc1;
        info << ",epc2=" << std::hex << rst_info->epc2;
        info << ",epc3=" << std::hex << rst_info->epc3;
        info << ",excvaddr=" << std::hex << rst_info->excvaddr;
        info << ",depc=" << std::hex << rst_info->depc;
        //The   address of  the last    crash   is  printed,    which   is  used    to debug garbled output.
    }
    info << std::endl;
    return info.str();
}
std::string to_string(uint32_t ul)
{
    char tt[20];
    snprintf(tt, sizeof(tt) - 1, "%u", ul);
    tt[sizeof(tt) - 1] = 0;
    return std::string(tt);
}
