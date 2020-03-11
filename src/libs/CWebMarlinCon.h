/*
 * CWebMarlinCon.h
 *
 *  Created on: Dec 29, 2019
 *      Author: family
 */

#ifndef LIBS_CWEBMARLINCON_H_
#define LIBS_CWEBMARLINCON_H_
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WString.h>
#include "CMarlinCon.h"
class CWebMarlinCon:public CMarlinCon_Listener {
    std::string m_cmd;
    ESP8266WebServer &server;
    virtual std::string getCmd();
public:
    CWebMarlinCon(ESP8266WebServer &server_);
    void handler();
};

#endif /* LIBS_CWEBMARLINCON_H_ */
