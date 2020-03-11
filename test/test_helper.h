/*
 * test_helper.h
 *
 *  Created on: 20 ñ³÷. 2020 ð.
 *      Author: ominenko
 */

#pragma once

#include "../src/libs/CMarlinCon.h"
#include <string>

using namespace std;

class CMarlinCon_implTest: public CMarlinCon {
public:
    string m_write;
    string m_read;
    void write(const std::string data) override
    {
        m_write = data;
    }

    std::string read() override
    {
        return m_read;
    }

};

class CMarlinCon_Listener_implTest: public CMarlinCon_Listener {
public:
    string m_cmd;
    std::string getMbuff()
    {
        return m_buff;
    }
    virtual std::string getCmd() override
    {
        auto cmd = m_cmd;
        m_cmd.clear();
        return cmd;
    }
};

