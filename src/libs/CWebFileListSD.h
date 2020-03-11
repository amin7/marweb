/*
 * CFileList.h
 *
 *  Created on: 13 ����. 2019 �.
 *      Author: ominenko
 */

#pragma once

#include <stdint.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <SdFat.h>
#include <memory>
#include "CManageSDControl.h"

class CWebFileListSD {
    ESP8266WebServer &server;
    sdfat::SdFat &sdFat;
    sdfat::SdFile m_uploadSdFile;
    CManageSDControl &m_sdCnt;
    public:
    void getFileList();
    void handleFileDownload();
    void handleFileUpload();
    CWebFileListSD(ESP8266WebServer &_server, sdfat::SdFat &_sdFat, CManageSDControl &sdCnt);
    static String normalizePath(const String &path_uri);
};

