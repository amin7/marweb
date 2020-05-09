#define DESTRUCTOR_CLOSES_FILE 1
#define SPIFFS_CACHE 7
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <sdios.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPUpdateServer.h>

#include "libs/CWebFileListSD.h"
#include "libs/CWebServer.h"
#include "libs/sdcard_ex.h"
#include "libs/misk.h"
#include "libs/logs.h"
#include "libs/StringStream.h"
#include "libs/CStatus.h"
#include "libs/CMarlinCon.h"
#include "libs/CWebMarlinCon.h"
#include "libs/CProbeArea.h"
#include "libs/CManageSDControl.h"
#include "libs/wifiHandle.h"

using namespace sdfat;

constexpr auto SERVER_PORT_WEB = 80;

constexpr auto pin_SD_CS = D2;
constexpr auto pin_MISO = D6;
constexpr auto pin_MOSI = D7;
constexpr auto pin_SCLK = D5;
constexpr auto pin_CARD_INSERTED = D0;
constexpr auto pin_START_DEFAULT_CFG = D3;

constexpr auto RXBUFFERSIZE = 1024;
constexpr auto SERIAL_BAUND = 115200;

constexpr auto DEF_DEVICE_NAME = "MarWeb"; //AP name
constexpr auto DEF_WIFI_MODE = WIFI_AP;
constexpr auto DEF_SSID_NAME = "";
constexpr auto DEF_WIFI_PWD = "12345678";

constexpr auto ota_update_path = "/firmware";
constexpr auto ota_username = "admin";
constexpr auto ota_password = "1234";

constexpr auto config_file = "/wifi.json";
constexpr auto config_file_spiffs = "/config/wifi.json";
constexpr auto txt_MarlinRead = "marlin read";

SdFat sdFat;
ESP8266WebServer serverWeb(SERVER_PORT_WEB);
ESP8266HTTPUpdateServer otaUpdater;
ArduinoOutStream out(Serial);
CWebMarlinCon WebMarlinCon(serverWeb);

class CMarlinCon_impl: public CMarlinCon
{
    static constexpr auto CMD_SZ = 124;
    virtual void write(const std::string data) override
    {
        Serial.println(data.c_str());
    }
    virtual std::string read() override
    {
        char buffer[CMD_SZ + 1];
        const auto read_sz = Serial.read(buffer, CMD_SZ);
        buffer[read_sz] = 0;
        return std::string(buffer);
    }
}MarlinCon;

class CManageSDControl_impl: public CManageSDControl, public CStatus {
    bool takeSD_impl() override
    {
        digitalWrite(pin_CARD_INSERTED, HIGH); //card_eject
        LED_ON();
        pinMode(pin_MISO, SPECIAL);
        pinMode(pin_MOSI, SPECIAL);
        pinMode(pin_SCLK, SPECIAL);
        pinMode(pin_SD_CS, OUTPUT);

        DBG_PRINTLN("takeBus");
        if (sdFat.begin(pin_SD_CS, SPI_FULL_SPEED))
        {
            DBG_PRINTLN("sd init ok");
            return true;
        }

#ifdef DEBUG_STREAM
        sdFat.initErrorPrint();
#endif
        ERR_LOG("sd initError");
        return false;
    }
    void returnSD_impl() override
    {
        DBG_PRINTLN("returnBus");
        pinMode(pin_MISO, INPUT);
        pinMode(pin_MOSI, INPUT);
        pinMode(pin_SCLK, INPUT);
        pinMode(pin_SD_CS, INPUT);
        LED_OFF();
        digitalWrite(pin_CARD_INSERTED, LOW); //card_insert
    }
public:

  void getStatus(JsonObject &root) const override
  {
        root["sdmode"] = static_cast<unsigned>(isOwned());
  }
} sdCnt;

CWebFileListSD FileList(serverWeb, sdFat, sdCnt);
CWebServer webHandelrs(serverWeb, sdCnt);
CMarlinEspCmd EspCmd;
// ------------------------

void http_about()
{
    DBG_PRINTLN("http_about ");
    serverWeb.setContentLength(CONTENT_LENGTH_UNKNOWN);
    serverWeb.sendHeader("Content-Type", "text/plain", true);
    auto about = String("");
    StringStream stream_about(about);
    ArduinoOutStream coutput(stream_about);

    coutput << "Compiled :" << __DATE__ << " " << __TIME__ << endl;
    coutput << "Serial Speed " << SERIAL_BAUND << " buff " << RXBUFFERSIZE << endl;
    coutput << getResetInfo().c_str() << endl;
    coutput << "RSSI " << WiFi.RSSI() << endl;
    
#ifdef DEBUG_STREAM
    coutput << "DEBUG_STREAM on" << endl;
#endif
    serverWeb.sendContent(about);
    about = "\n";
    hw_info(stream_about);
    serverWeb.sendContent(about);
    about = "\n";
    SPIFFS_info(stream_about);
    serverWeb.sendContent(about);
    about = "\n";
    coutput << "isOwnedSD " << static_cast<unsigned>(sdCnt.isOwned()) << endl;
    if (sdCnt.isOwned())
    {
        sdcard_info(sdFat, stream_about);
    } else
    {
        about += txt_MarlinRead;
    }

    serverWeb.sendContent(about);

    //---------------
    serverWeb.sendContent("");
}
void http_status() {
    DBG_PRINTLN("http_status ");
    serverWeb.setContentLength(CONTENT_LENGTH_UNKNOWN);
    serverWeb.sendHeader("Content-Type", "application/json", true);
    serverWeb.sendHeader("Cache-Control", "no-cache");
    //---------------
    StaticJsonDocument<500> status;
    auto object = status.to<JsonObject>();
    auto json_dav = object.createNestedObject("dav");
    sdCnt.getStatus(json_dav);
    auto json_ProbeArea = object.createNestedObject("probe");
    webHandelrs.getStatus(json_ProbeArea);
    serverWeb.sendContent(status.as<String>());
    serverWeb.sendContent("");
    DBG_PRINT("memoryUsage ");
    DBG_PRINTLN(status.memoryUsage());

}

void setStateSDcontrol() {
    DBG_PRINTLN("/setStateSDcontrol");
    if (serverWeb.hasArg("mode"))
    {
        const auto mode = static_cast<bool>(serverWeb.arg("mode").toInt());
        DBG_PRINT("mode ");
        DBG_PRINTLN(mode);
        if (mode) {
            sdCnt.takeSD();
        } else {
            sdCnt.returnSD();
        }
    }
    http_status();
}
void setupWeb()
{
    serverWeb.on("/command", HTTP_ANY,
            [&]()
            {
                WebMarlinCon.handler();
            });
    serverWeb.on("/gcodeInfo", HTTP_ANY,
            [&]()
            {
                webHandelrs.handleGetGCodeInfo();
            });
    serverWeb.on("/about", HTTP_ANY, http_about);
    serverWeb.on("/status", HTTP_ANY, http_status);
    serverWeb.on("/setStateSDcontrol", setStateSDcontrol);
    serverWeb.on("/probe", HTTP_ANY,
            [&]()
            {
                webHandelrs.handleProbes();
            });
    serverWeb.on("/levelmod", HTTP_ANY,
            [&]()
            {
                webHandelrs.handleLevelMod();
            });
    serverWeb.on("/filelist", HTTP_ANY,
            [&]()
            {
                FileList.getFileList();
            },
            [&]()
            {
                FileList.handleFileUpload();
            });
    serverWeb.on("/filedownload", HTTP_ANY,
            [&]()
            {
                FileList.handleFileDownload();
            });
    serverWeb.on("/scanwifi", HTTP_ANY,
            [&]()
            {
    		wifiHandle_sendlist(serverWeb);
            });
    serverWeb.on("/connectwifi", HTTP_ANY,
            [&]()
            {
    		wifiHandle_connect(serverWeb);
            });

    serverWeb.onNotFound([&]()
            {
            webHandelrs.handleFile();
            });

}
#ifndef UNIT_TEST
void setup() {
    pinMode(pin_START_DEFAULT_CFG, INPUT);
    pinMode(pin_CARD_INSERTED, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(SERIAL_BAUND);
    Serial.setRxBufferSize(RXBUFFERSIZE);
    sdCnt.setup();
   // setup_wifi(DEF_SSID_NAME, DEF_WIFI_PWD, DEF_DEVICE_NAME, DEF_WIFI_MODE);
    SPIFFS.begin();
    MDNS.begin(WiFi.hostname().c_str());
    otaUpdater.setup(&serverWeb, ota_update_path, ota_username, ota_password);
    setupWeb();
    MDNS.addService("http", "tcp", SERVER_PORT_WEB);

    EspCmd.addHandler("// action:setStateSDcontrolMarlin\r", [] {
        if (sdCnt.isOwned()) {
            sdCnt.returnSD();
        }
    });

    EspCmd.addHandler("// action:resetwifi\r", [] {
        DBG_PRINTLN("resetwifi");
    	setup_wifi(DEF_SSID_NAME, DEF_WIFI_PWD, DEF_DEVICE_NAME, DEF_WIFI_MODE);
    });

    MarlinCon.addListener(WebMarlinCon);
    MarlinCon.addListener(webHandelrs.m_ProbeArea);
    MarlinCon.addListener(EspCmd);

    // ----- can't send to uartd arduino mega before. it can cause to activate bootloader
#ifdef DEBUG_STREAM
    SPIFFS_info(DEBUG_STREAM);
    sdcard_info(sdFat, DEBUG_STREAM);
#endif
    serverWeb.begin();
    out << F(";servers started") << endl;
    WiFi.begin();
    //print IP
    out << F("M117 ");
    if (WIFI_AP == WiFi.getMode())
    {
        out << F("AP ") <<WiFi.hostname()<<" "<< WiFi.softAPIP();
    } else
    {
    	 out << F("conn.. ")<<WiFi.SSID();
    }
    out << endl;
}

// ------------------------
void loop() {
    MDNS.update();
    serverWeb.handleClient();
    MarlinCon.loop();
    wifiHandle_loop();
}
#endif
