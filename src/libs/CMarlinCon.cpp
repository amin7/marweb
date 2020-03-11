/*
 * CMarlinCon.cpp
 *
 *  Created on: Dec 29, 2019
 *      Author: family
 */

#include "CMarlinCon.h"

using namespace std;

void CMarlinCon::addListener(CMarlinCon_Listener_IF &listener)
{
  m_listeners.push_back(&listener);
}

void CMarlinCon::loop(){
    auto in = read();
    while (in.length())
    {
        auto lineEnd = in.find_first_of('\n');
        m_line += in.substr(0, lineEnd);
        if (string::npos != lineEnd)
        {
            //catch \n
            for (auto listener : m_listeners)
            {
                listener->pushLine(m_line);
            }
            m_line.clear();
            in = in.substr(lineEnd + 1);
        } else
        {
            break;
        }
    }
    if (m_line.empty())
    {
        std::string cmd;
        for (auto listener : m_listeners)
        {
            cmd = listener->getCmd();
            if (!cmd.empty()) {
                break;

            }
        }
        if (!cmd.empty())
        {
            write(cmd);
            string echo = ">>";
            echo += cmd; //echo
            for (auto listener : m_listeners)
            {
                listener->pushLine(echo);
            }
        }
    }
}

void CMarlinCon_Listener::pushLine(const std::string &line){
  if((m_buff.length()+line.length())>(MAX_BUFF_SZ-1)){
    m_buff.clear();
  }
  m_buff+=line+'\n';
}
