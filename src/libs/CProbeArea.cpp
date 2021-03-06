/*
 * CProbeArea.cpp
 *
 *  Created on: Jan 11, 2020
 *      Author: family
 */

#include "CProbeArea.h"
#include "logs.h"
#include <sstream>
#include <iomanip>
using namespace std;

void CProbeArea::stop()
{
    clear();
    addCmd(end_gcode);
    addCmd(end_msg_stop);
}

bool CProbeArea::isErrorResult(const string &result)
{
    const auto is_error = result.find("Error");
    if (string::npos == is_error)
    {
        return false;
    }
    return true;
}
//-------------------------------------------
double CProbeArea::getZ(const string &result)
{
    const auto zpos = result.find("Z:");
    if (string::npos == zpos)
    {
        return nan("");
    }
    const auto posLF = result.find('\n', zpos + 2);
    auto val = result.substr(zpos + 2, (string::npos == posLF) ? posLF : (posLF - zpos - 2));
    const auto posSpace = val.find(' ');
    if (string::npos != posSpace)
    {
        val.erase(posSpace);
    }

    if (val.empty())
    {
        return nan("");
    }

#ifdef UNIT_TEST
    return stod(val);
#else
    return String(val.c_str()).toDouble();
#endif
}


bool CProbeArea::positionCallback(string &result)
{
    do
    {
        if (isErrorResult(result))
        {
            break;
        }
        const auto zval = getZ(result);
        if (isnan(zval))
        {
            break;
        }
        m_AreaGenerator.setZheigh(zval);
        int16_t dX, dY;
        if (m_AreaGenerator.getNext(dX, dY))
        {
            ostringstream os_cmd;
            os_cmd << "M117 probing:" << static_cast<unsigned>(m_AreaGenerator.getDone()) << "/" << static_cast<unsigned>(m_AreaGenerator.getTotal()) << "\n";
            os_cmd << "G0 F" << m_feedRateProbe * 2 << " Z" << m_levelDelta << "\n";
            os_cmd << "G0 F" << m_feedRateXY << " X" << dX << " " << " Y" << dY << "\n";
            os_cmd << "G38.2 F" << m_feedRateProbe << " Z" << m_levelDelta * -2;
            if (addCmd(os_cmd.str(),
                    [&](string &result) -> bool
                    {
                        if (isErrorResult(result))
                        {
                            stop();
                            setMode(paError);
                            return false;
                        }
                        return addCmd(gcode_GetCurrentPosition,
                                [&](string &result) -> bool
                                {
                                    return positionCallback(result);
                                });
                    }))
            {
                return true;
            }
        } else
        {
            addCmd(end_gcode);
            addCmd(end_msg_done);
            setMode(paDone);
            return true;
        }
    } while (0);
    stop();
    setMode(paError);
    return false;
}

bool CProbeArea::multipleCB(string &result)
        {
    do
    {
        const auto zval = getZ(result);
        if (isnan(zval))
                {
            break;
        }
        ostringstream os_cmd;
        os_cmd << "M117 delta:" << std::fixed << std::setprecision(2) << zval << "\n";
        os_cmd << "G0 F" << m_feedRateProbe * 2 << " Z" << m_levelDelta << "\n";
        os_cmd << "G38.2 F" << m_feedRateProbe << " Z" << m_levelDelta * -2;
        if (addCmd(os_cmd.str(),
                [&](string &result) -> bool
                {
                    if (isErrorResult(result))
                            {
                        stop();
                        setMode(paError);
                        return false;
                    }
                    return addCmd(gcode_GetCurrentPosition,
                            [&](string &result) -> bool
                            {
                                return multipleCB(result);
                            });
                }))
        {
            return true;
        }

    } while (0);
    stop();
    setMode(paError);
    return false;
}


bool CProbeArea::run(uint16_t sizeX, uint16_t sizeY, uint16_t grid, double levelDelta, uint16_t feedRateXY, uint16_t feedRateProbe, bool doubleTouch)
{
    if (getMode() == paRun)
    {
        return false; //incorrect mode
    }
    if (0 == grid) {
        return false;
    }
    
    if (!m_AreaGenerator.init(sizeX, sizeY, grid))
    {
        return false;
    }
    clear();

    m_levelDelta = levelDelta;
    m_feedRateXY = feedRateXY;
    m_feedRateProbe = feedRateProbe;
//first
    ostringstream os_cmd;
    os_cmd << beg_gcode << "\n";
    os_cmd << "G21\nG91\n";
    os_cmd << "G0 F" << m_feedRateProbe * 2 << " Z" << probe_first_distance << "\n";
    os_cmd << gcode_EnableSteppers << "\n";
    os_cmd << "G38.2 F" << m_feedRateProbe << " Z" << probe_first_distance * -2;
    if (!addCmd(os_cmd.str(),
            [&](string &result) -> bool
            {
                if (isErrorResult(result))
                {
                    stop();
                    setMode(paError);
                    return false;
                }
                return addCmd("G92 X0 Y0 Z0",  //return as M114
                        [&](string &result) -> bool
                        {
                            return positionCallback(result);
                        });
            }
    )) //relative pos    +mm    unit
    {
        return false;
    }
    setMode(paRun);
    return true;
}

bool CProbeArea::multiple(double levelDelta, uint16_t feedRateProbe) {
    if (getMode() == paRun)
            {
        return false; //incorrect mode
    }
    clear();

    m_levelDelta = levelDelta;
    m_feedRateProbe = feedRateProbe;
//first
    ostringstream os_cmd;
    os_cmd << beg_gcode << "\n";
    os_cmd << "G21\nG91\n";
    os_cmd << gcode_GetCurrentPosition;
    if (!addCmd(os_cmd.str(),
    [&](string &result) -> bool
    {
        return multipleCB(result);
    })) {
        return false;
    }
    setMode(paRun);
    return true;
}

