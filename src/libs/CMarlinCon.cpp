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
    const auto in = read();
    std::string::size_type Pos = 0;
    while (Pos < in.length())
    {
        const auto lineEnd = in.find_first_of('\n', Pos);
        if (string::npos != lineEnd)
        {
            m_line.append(in.begin() + Pos, in.begin() + lineEnd);
            for (auto listener : m_listeners)
            {
                listener->pushLine(m_line);
            }
            m_line.clear();
            Pos = lineEnd + 1;
        } else
        {
            m_line.append(in.begin() + Pos, in.end());
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
