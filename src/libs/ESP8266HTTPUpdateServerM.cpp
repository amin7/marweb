#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include "StreamString.h"
#include "logs.h"

#include "ESP8266HTTPUpdateServerM.h"
#include "ESP8266HTTPUpdateServerM_html.h"

static const char successResponse[] PROGMEM = 
  "<META http-equiv=\"refresh\" content=\"15;URL=/\">Update Success! Rebooting...\n";

ESP8266HTTPUpdateServerM::ESP8266HTTPUpdateServerM(bool serial_debug)
{
  _server = NULL;
  _username = emptyString;
  _password = emptyString;
  _authenticated = false;
}

void ESP8266HTTPUpdateServerM::setup(ESP8266WebServer *server, const String& path, const String& username, const String& password)
{
    _server = server;
    _username = username;
    _password = password;

    // handler for the /update form page
    _server->on(path.c_str(), HTTP_GET, [&](){
      if(_username != emptyString && _password != emptyString && !_server->authenticate(_username.c_str(), _password.c_str()))
        return _server->requestAuthentication();
        _server->send_P(200, PSTR("text/html"), ____frontend_ESP8266HTTPUpdateServerM_html_);
    });

    // handler for the /update form POST (once file upload finishes)
    _server->on(path.c_str(), HTTP_POST, [&](){
        if(!_authenticated)
        {
            return _server->requestAuthentication();
        }
      if (Update.hasError()) {
        _server->send(200, F("text/html"), String(F("Update error: ")) + _updaterError);
      } else {
        _server->client().setNoDelay(true);
        _server->send_P(200, PSTR("text/html"), successResponse);
        delay(100);
        _server->client().stop();
        ESP.restart();
      }
    },[&](){
      // handler for the file upload, get's the sketch bytes, and writes
      // them through the Update object
      HTTPUpload& upload = _server->upload();

      if(upload.status == UPLOAD_FILE_START){
            _updaterError = String();
            _authenticated = (_username == emptyString || _password == emptyString || _server->authenticate(_username.c_str(), _password.c_str()));
            if(!_authenticated)
            {
                return;
            }
            WiFiUDP::stopAll();
            const auto _command = _server->arg("cmd").toInt();
            const uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
            DBG_PRINT("U cmd=");
            DBG_PRINTLN(_command);
            DBG_PRINT("maxSketchSpace=");
            DBG_PRINTLN(maxSketchSpace);

            if(!Update.begin(maxSketchSpace,_command,LED_BUILTIN))
            {      //start with max available size
                _setUpdaterError();
            }
        } else if(_authenticated && upload.status == UPLOAD_FILE_WRITE && !_updaterError.length())
        {
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
          _setUpdaterError();
        }
      } else if(_authenticated && upload.status == UPLOAD_FILE_END && !_updaterError.length()){
        if(Update.end(true)){ //true to set the size to the current progress
        } else {
          _setUpdaterError();
        }
      } else if(_authenticated && upload.status == UPLOAD_FILE_ABORTED){
        Update.end();
      }
      delay(0);
    });
}

void ESP8266HTTPUpdateServerM::_setUpdaterError()
{
  StreamString str;
  Update.printError(str);
  _updaterError = str.c_str();
}
