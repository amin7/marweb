/*
 * CMarlinCon.h
 *
 *  Created on: Dec 29, 2019
 *      Author: family
 */

#pragma once
#include <map>
#include <vector>
#include <string>
#include <functional>
#include "misk.h"
#include "logs.h"
#ifdef UNIT_TEST
#include "gtest/gtest.h"
#endif

class CMarlinCon_Listener_IF {
public:
    virtual void pushLine(const std::string &line)=0;
    virtual std::string getCmd() =0;
    virtual ~CMarlinCon_Listener_IF()
    {
    }
    ;
};

class CMarlinCon {
    std::string m_line;
    std::vector<CMarlinCon_Listener_IF*> m_listeners;
public:
    virtual ~CMarlinCon(){}
    void addListener(CMarlinCon_Listener_IF &listener);
    void loop();
    virtual void write(const std::string data)=0;
    virtual std::string read()=0;
};

class CMarlinCon_Listener: public CMarlinCon_Listener_IF {
  static constexpr auto MAX_BUFF_SZ=1024;
protected:
    std::string m_buff;
public:
    virtual void pushLine(const std::string &line);
};

class CMarlinEspCmd: public CMarlinCon_Listener_IF {
    std::map<const char*, std::function<void()>, CompareCStrings> m_EspCmd;
    void pushLine(const std::string &line) {
        auto handler = m_EspCmd.find(line.c_str());
        if (m_EspCmd.end() != handler)
                {
            DBG_PRINT(" handlers:");
            DBG_PRINTLN(line.c_str());
            handler->second();
        }
        }
    std::string getCmd() {
        return "";
    }
public:
    void addHandler(const char *cmd, std::function<void()> handler) {
        m_EspCmd[cmd] = handler;
    }
};

