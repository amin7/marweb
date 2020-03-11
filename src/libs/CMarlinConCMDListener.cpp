/*
 * CMarlinConCMDListener.cpp
 *
 *  Created on: 23 ñ³÷. 2020 ð.
 *      Author: ominenko
 */

#include "CMarlinConCMDListener.h"

#include "CGcodeParser.h"
using namespace std;

CMarlinConCMD_Listener::CMarlinConCMD_Listener() :
        m_waitOkCount(0)
{
}

void CMarlinConCMD_Listener::clear()
{
    m_cmdList = { };
}

void CMarlinConCMD_Listener::pushLine(const std::string &line)
{
    CMarlinCon_Listener::pushLine(line);
    if (line.rfind("ok") == 0)
    {
        if (m_waitOkCount)
        {
            auto handler = m_cmdList.front();
            m_cmdList.pop();
            if (nullptr != handler.responce)
            {
                handler.responce(m_buff);
                m_buff.clear();
            }
            m_waitOkCount--;
        }
    }
}

std::string CMarlinConCMD_Listener::getCmd()
{
    if ((!m_cmdList.empty()) && (0 == m_waitOkCount))
    {
        auto handler = m_cmdList.front();
        m_waitOkCount++;
        m_buff.clear();
        return handler.cmd;
    }
    return "";
}
bool CMarlinConCMD_Listener::addCmd(const std::string &cmd, std::function<bool(std::string &result)> responce)
{
    auto arr = normalizeGCode(cmd);

    const auto sz = arr.size();
    if (!sz) {
        return true;
    }
    tMarlinCMD handler;
    handler.responce = nullptr;
    for (size_t i = 0; i < sz; i++) {
        handler.cmd = arr.at(i);
        if (i == (sz - 1)) {
            handler.responce = responce;
        }
        m_cmdList.push(handler);
    }
    return true;
}

bool CMarlinConCMD_Listener::addCmd(const string &cmd)
{
    return addCmd(cmd, nullptr);
}
