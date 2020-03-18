#define DESTRUCTOR_CLOSES_FILE 1
#include <arduino.h>
#include <ESP8266WiFi.h>
#include <sdios.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPUpdateServer.h>

#include "libs\CWebFileListSD.h"
#include "libs\CWebServer.h"
#include "libs\sdcard_ex.h"
#include "libs\misk.h"
#include "libs\logs.h"
#include "libs\StringStream.h"
#include "libs\CStatus.h"
#include "libs\CMarlinCon.h"
#include "libs\CWebMarlinCon.h"
#include "libs\CProbeArea.h"
#include "libs\CManageSDControl.h"

using namespace sdfat;

constexpr auto SERVER_PORT_WEB = 80;

constexpr auto pin_SD_CS = D2;
constexpr auto pin_MISO = D6;
constexpr auto pin_MOSI = D7;
constexpr auto pin_SCLK = D5;
constexpr auto pin_CS_SENSE = D1;
constexpr auto pin_CARD_INSERTED = D0;
constexpr auto pin_START_DEFAULT_CFG = D3;

constexpr auto RXBUFFERSIZE = 1024;
constexpr auto SERIAL_BAUND = 115200;

constexpr auto DEF_DEVICE_NAME = "WebDAV"; //AP name
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
    virtual void write(const std::string data) override
    {
        Serial.println(data.c_str());
    }
    virtual std::string read() override
    {
        std::string data;
        while (Serial.available())
        {
            data += Serial.read();
        }
        return data;
    }
}MarlinCon;

class CManageSDControl_impl: public CManageSDControl,public CStatus {
    bool takeBus()
    {
        digitalWrite(pin_CARD_INSERTED, HIGH); //card_eject
        LED_ON();
        pinMode(pin_MISO, SPECIAL);
        pinMode(pin_MOSI, SPECIAL);
        pinMode(pin_SCLK, SPECIAL);
        pinMode(pin_SD_CS, OUTPUT);

        DBG_PRINTLN("sd begin..");
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
    void returnBus()
    {
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
    root["sdmode"] = static_cast<unsigned>(getStateSDcontrol());
  }
} sdCnt;

CWebFileListSD FileList(serverWeb, sdFat, sdCnt);
CWebServer webHandelrs(serverWeb, sdCnt);
// ------------------------

ICACHE_RAM_ATTR void cs_sense_isr()
{
    sdCnt.cs_marlin_isr();
}

void get_configs()
{
    auto wifi_host_name_ = (String) DEF_DEVICE_NAME;
    auto wifi_pwd_ = (String) DEF_WIFI_PWD;
    auto wifi_mode_ = DEF_WIFI_MODE;
    auto wifi_ssid_ = (String) DEF_SSID_NAME;
    unsigned long to = 0;
    while (sdCnt.isMarlinUseSPI())
    {
        yield();
#ifdef DEBUG_STREAM
        if (millis() > to)
        {
            to = millis() + 300;
            DBG_PRINT('.');
        }
#endif
    }
    DBG_PRINTLN();
    sdCnt.requestSDcontrol();
    DBG_PRINT(F("config_file "));
    DBG_PRINTLN(config_file);

    auto wifi_conf = sdFat.open(config_file);
    // check for open error
    if (wifi_conf)
    {
        get_config(wifi_conf, wifi_ssid_, wifi_pwd_, wifi_host_name_, wifi_mode_);
        wifi_conf.close();
    } else
    {
        DBG_PRINTLN(F("SPIFFS.open "));
        auto wifi_conf_spiffs = SPIFFS.open(config_file_spiffs, "r");
        if (wifi_conf_spiffs)
        {
            get_config(wifi_conf_spiffs, wifi_ssid_, wifi_pwd_, wifi_host_name_, wifi_mode_);
            wifi_conf_spiffs.close();
        } else
        {
            ERR_LOG(F("no wifiConfig"));
        }
    }

    setup_wifi(wifi_ssid_, wifi_pwd_, wifi_host_name_, wifi_mode_);
    MDNS.begin(wifi_host_name_.c_str());
}

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
    coutput << "getStateSDcontrol " << static_cast<unsigned>(sdCnt.getStateSDcontrol()) << endl;
    if (sdCnt.requestSDcontrol())
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
}

void setStateSDcontrol() {
    DBG_PRINTLN("/setStateSDcontrol");
    if (serverWeb.hasArg("mode"))
    {
        const auto mode = static_cast<tStateSDcontrol>(serverWeb.arg("mode").toInt());
        DBG_PRINT("mode ");
        DBG_PRINTLN(mode);
        sdCnt.setStateSDcontrol(mode);
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

    serverWeb.onNotFound([&]()
            {
                webHandelrs.handleFile();
            });

}
#ifndef UNIT_TEST
void setup() {
    // ------------------------
    // ----- GPIO -------
    // Detect when other master uses SPI bus
    pinMode(pin_CS_SENSE, INPUT);
    pinMode(pin_START_DEFAULT_CFG, INPUT);
    pinMode(pin_CARD_INSERTED, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    attachInterrupt(pin_CS_SENSE, cs_sense_isr, RISING);// marlin release CS
    Serial.begin(SERIAL_BAUND);
    Serial.setRxBufferSize(RXBUFFERSIZE);
    SPIFFS.begin();
    sdCnt.setup();
    get_configs();

    otaUpdater.setup(&serverWeb, ota_update_path, ota_username, ota_password);
    setupWeb();
    MDNS.addService("http", "tcp", SERVER_PORT_WEB);

    MarlinCon.addListener(WebMarlinCon);
    MarlinCon.addListener(webHandelrs.m_ProbeArea);

    // ----- can't send to uartd arduino mega before. it can cause to activate bootloader
#ifdef DEBUG_STREAM
    SPIFFS_info(DEBUG_STREAM);
    sdcard_info(sdFat, DEBUG_STREAM);
#endif
    serverWeb.begin();
    out << F(";servers started") << endl;
    out << F(";SERVER_PORT_WEB:") << SERVER_PORT_WEB << endl;
    //print IP
    out << F("M117 ");
    if (WIFI_AP == WiFi.getMode())
    {
        out << F("AP ") << WiFi.softAPIP();
    } else
    {
        out << WiFi.localIP();
    }
    out << ":" << SERVER_PORT_WEB << endl;
}

// ------------------------
void loop() {
    MDNS.update();
    sdCnt.loop();
    serverWeb.handleClient();
    MarlinCon.loop();
}
#endif
