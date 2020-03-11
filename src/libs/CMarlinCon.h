/*
 * CMarlinCon.h
 *
 *  Created on: Dec 29, 2019
 *      Author: family
 */

#pragma once
#include <vector>
#include <string>
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

