/*
 * CGcodeParser.cpp
 *
 *  Created on: 19 груд. 2019 р.
 *      Author: ominenko
 */

#include "CGcodeParser.h"

#include "Logs.h"
#include <iostream>
#include <string>
#include <stdlib.h>
#include <sstream>
#include <iomanip>

using namespace std;
/*
 * remove comment, trail space
 * and split on substring
 */
std::vector<string> normalizeGCode(const string &inp) {
    std::vector < string > res;
    string::size_type cmdBeg = 0;
    string::size_type endLine;
    do
    {
        cmdBeg = inp.find_first_not_of(' ', cmdBeg);
        if (string::npos == cmdBeg) {
            break;
        }
        endLine = inp.find('\n', cmdBeg);
        auto cmd = inp.substr(cmdBeg, (endLine == string::npos) ? string::npos : (endLine - cmdBeg));
        const auto endCmd = cmd.find_first_of(';');
        if (string::npos != endCmd) {
            cmd.erase(endCmd);
        }
        if (!cmd.empty()) {
            res.push_back(cmd);
        }
        cmdBeg = endLine + 1;
    } while (endLine != string::npos);
    return res;
}

bool CGcodeParser::addCmd(const string &inp)
{
    auto arr = normalizeGCode(inp);
    for (auto line : arr) {
        lineTotal++;
        DBG_PRINT(lineTotal);
        istringstream iLine(line);

        string cmd;
        if (!getline(iLine, cmd, ' '))
                {
            DBG_PRINTLN("no cmd");
            return false;
        }

        DBG_PRINTLN(cmd.c_str());

        tGCodeParams paramIn;
        string paramLn;
        while (getline(iLine, paramLn, ' '))
        {
            if (paramLn.empty())
            {
                continue;
            }
            double value = nan("");
            const char name = paramLn.at(0);
            if ('A' > name || 'Z' < name)
                    {            // A-Z expected
                return false;
            }

            if (1 < paramLn.size())
                    {
#ifdef UNIT_TEST
                value = stod(paramLn.substr(1));
    #else
                value = String(paramLn.substr(1).c_str()).toDouble();
#endif
            }

            paramIn[name] = value;
        }

#ifdef DEBUG_STREAM
        for (auto item : paramIn)
        {
            DBG_PRINT(item.first);
            DBG_PRINT(':');
            DBG_PRINT(item.second);
            DBG_PRINT(' ');
        }
        DBG_PRINTLN(' ');
    #endif

        for (auto handler : m_GCodeHandlers)
        {
            if (!handler(cmd.c_str(), paramIn))
            {
                DBG_FUNK_LINE();
                DBG_PRINTLN(" handlers err");
                return false;
            }
        }
        lineProcessed++;
    }
    return true;
}
void CGcodeParser::addHandler(tGCodeHandler handler)
{
    m_GCodeHandlers.push_back(handler);
}
//Linear Move
bool handler_G0_G1(const tGCodeParams &in, tControlState &out)
{
    auto deltaPos = CPoint(nan(""));
    for (const auto attr : in)
    {
        const auto val = attr.second;
        if (isnan(val))
        {
            return false;
        }
        switch (attr.first) {
            case 'X':
                deltaPos.setX(attr.second);
                break;
            case 'Y':
                deltaPos.setY(attr.second);
                break;
            case 'Z':
                deltaPos.setZ(attr.second);
                break;
            case 'F':
                out.feed = val;
                break;
        }
    }
    if (out.isAbsolutePositioning)
    {
        out.point.copyNNN(deltaPos);
    } else
    {
        out.point += deltaPos;
    }
    return true;
}
//Millimeter Units
bool handler_G21(const tGCodeParams &in, tControlState &out)
{
    return true;
}
//Absolute Positioning
bool handler_G90(const tGCodeParams &in, tControlState &out)
{
    out.isAbsolutePositioning = true;
    return true;
}
//Relative Positioning
bool handler_G91(const tGCodeParams &in, tControlState &out)
{
    out.isAbsolutePositioning = false;
    return true;
}
//Auto Home
bool handler_G28(const tGCodeParams &in, tControlState &out)
{
    if (in.empty())
    {
        out.point = { 0 };
        return true;
    }
    auto newPos = CPoint(nan(""));
    for (const auto attr : in)
    {
        switch (attr.first) {
            case 'X':
            newPos.setX(0);
                break;
            case 'Y':
            newPos.setY(0);
                break;
            case 'Z':
            newPos.setZ(0);
                break;
        }
    }
    out.point = newPos;
    return true;
}

void CGcodeParserSTD::clear()
{
    m_state = { };
}

CGcodeParserSTD::CGcodeParserSTD(CGcodeParser &parcer)
{
    m_cmdHandlers.clear();
    m_cmdHandlers["G0"] = handler_G0_G1;
    m_cmdHandlers["G1"] = handler_G0_G1;
    m_cmdHandlers["G21"] = handler_G21;
    m_cmdHandlers["G90"] = handler_G90;
    m_cmdHandlers["G91"] = handler_G91;
    m_cmdHandlers["G28"] = handler_G28;
    parcer.addHandler([&](const string &cmd, const tGCodeParams &params) -> bool
    {
        auto handler = m_cmdHandlers.find(cmd.c_str());
        if (m_cmdHandlers.end() == handler)
        {
            DBG_PRINT(" handlers not found:");
            DBG_PRINTLN(cmd.c_str());
            return true;
        }

        if (!handler->second(params, m_state))
        {
            DBG_FUNK_LINE();
            DBG_PRINTLN(" handlers err");
            return false;
        }
        return true;
    });
}

CGcodeInfo::CGcodeInfo(CGcodeParser &parcer) :
        CGcodeParserSTD(parcer)
{
    parcer.addHandler([&](const string &cmd, const tGCodeParams &params) -> bool
    {
        if (m_state.point.getX() < min.getX())
        {
            min.setX(m_state.point.getX());
        }
        if (m_state.point.getY() < min.getY())
        {
            min.setY(m_state.point.getY());
        }
        if (m_state.point.getZ() < min.getZ())
        {
            min.setZ(m_state.point.getZ());
        }
        if (m_state.point.getX() > max.getX())
        {
            max.setX(m_state.point.getX());
        }
        if (m_state.point.getY() > max.getY())
        {
            max.setY(m_state.point.getY());
        }
        if (m_state.point.getZ() > max.getZ())
        {
            max.setZ(m_state.point.getZ());
        }
        return true;
    });
}

CGcodeProducer::CGcodeProducer(CGcodeParser &parcer,std::function<bool(const std::string &str)> func):CGcodeParserSTD(parcer),m_write(func){
    parcer.addHandler([&](const string &cmd, const tGCodeParams &params) -> bool
            {
                return produceCmd(cmd,params);
            });
}

bool CGcodeProducer::add_comment(const std::string &comment)
{
    if (print_comment)
    {
        if (!m_write(";"))
        {
            return false;
        }
        if (!m_write(comment))
        {
            return false;
        }
        return m_write("\n");
    } else
    {
        return true;
    }
}
bool CGcodeProducer::produceCmd(const std::string &cmd, const tGCodeParams &par)
{
    ostringstream line_os;
    line_os << cmd;
    for (const auto attr : par)
    {
        line_os << " " << attr.first;
        if (!isnan(attr.second))
        {
            line_os << round(attr.second*precision_mul) / precision_mul;
        }
    }
    line_os << '\n';
    DBG_PRINT(">>");
    DBG_PRINT(line_os.str().c_str());
    return m_write(line_os.str());
}


