/*
 * FSBrowser.h
 *
 *  Created on: 24 זמגע. 2019 נ.
 *      Author: ominenko
 */

#pragma once
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <WString.h>
#include "CStatus.h"
#include "CProbeArea.h"
#include "CManageSDControl.h"

class CWebServer: public CStatus {
    ESP8266WebServer &m_server;
    CManageSDControl &m_sdCnt;
    bool handleFileRead(const String &path);
    public:
    CProbeArea m_ProbeArea;
    void handleProbesGet();
  CWebServer(ESP8266WebServer &server_, CManageSDControl &sdCnt) :
      m_server(server_), m_sdCnt(sdCnt)
    {
    }
    ;
    void handleFile();
    void handleProbes();
    void handleGetGCodeInfo();
    void handleLevelMod();
    void getStatus(JsonObject &root) const override;
};
