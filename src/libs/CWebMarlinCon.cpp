/*
 * CWebMarlinCon.cpp
 *
 *  Created on: Dec 29, 2019
 *      Author: family
 */

#include "CWebMarlinCon.h"
#include "logs.h"

std::string CWebMarlinCon::getCmd()
{
    auto cmd = m_cmd;
    m_cmd.clear();
    return cmd;
}

void CWebMarlinCon::handler()
{
    //DBG_PRINT("cs. ");

    if (server.hasArg("commandText"))
    {
      const auto cmd = server.arg("commandText");
      if (cmd != "")
      { //handle command
          DBG_PRINT("Web Command:");
          DBG_PRINTLN(cmd);
          m_cmd=cmd.c_str();

      }
    }

    server.sendHeader("Cache-Control", "no-cache");
    server.send(200, "text/plain", m_buff.c_str());
    m_buff.clear();
}

CWebMarlinCon::CWebMarlinCon(ESP8266WebServer &server_): server(server_)
{

}

