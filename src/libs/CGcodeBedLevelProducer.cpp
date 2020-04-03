/*
 * CGcodeBedLevelProducer.cpp
 *
 *  Created on: 2 бер. 2020 р.
 *      Author: ominenko
 */

#include <math.h>
#include "CGcodeBedLevelProducer.h"
#include "logs.h"

CGcodeBedLevelProducer::CGcodeBedLevelProducer(CGcodeParser &parcer, std::function<bool(const std::string &str)> writefn,
        CBedLevel &&BedLevel) :
        CGcodeProducer(parcer, writefn), m_BedLevel(BedLevel)
{

}

CGcodeBedLevelProducer::CGcodeBedLevelProducer(CGcodeParser &parcer, std::function<bool(const std::string &str)> writefn,
        const CProbeArea &probeArea) :
CGcodeBedLevelProducer(parcer, writefn,
                CBedLevel(probeArea.getSizeX(), probeArea.getSizeY(), probeArea.getGrid(), probeArea.getZheighArray()))
{
}
bool CGcodeBedLevelProducer::produceLine(const std::string &cmd, const tGCodeParams &par)
{
    auto par_loc = par;
    DBG_PRINT(m_state.point.getX());
    DBG_PRINT(" ");
    DBG_PRINT(m_state.point.getY());
    DBG_PRINT(" ");
    DBG_PRINT(m_state.point.getZ());
    DBG_PRINT(" ");
    DBG_PRINTLN(m_BedLevel.isInitOk());

    bool isXpresent = false;
    bool isYpresent = false;
    for (const auto attr : par)
    {
        if (!isnan(attr.second))
                {
            if ('X' == attr.first) {
                isXpresent = true;
            } else if ('Y' == attr.first) {
                isYpresent = true;
            }
        }
    }
    if (!isXpresent) {
        par_loc.erase('X');
    }
    if (!isYpresent) {
        par_loc.erase('Y');
    }

    auto beg_x = m_pre_state.point.getX();
    auto beg_y = m_pre_state.point.getY();
    auto beg_z = m_pre_state.point.getZ();

    do
    {
        const auto len = hypot(beg_x - m_state.point.getX(), beg_y - m_state.point.getY());
        if (len <= m_BedLevel.getGrid())
        { //last
            beg_x = m_state.point.getX();
            beg_y = m_state.point.getY();
            beg_z = m_state.point.getZ();
        } else if (len <= m_BedLevel.getGrid() * 2)
        { //divide on 2
            beg_x += (m_state.point.getX() - beg_x) / 2;
            beg_y += (m_state.point.getY() - beg_y) / 2;
            beg_z += (m_state.point.getZ() - beg_z) / 2;
        } else
        { // step on grid
            beg_x += (m_state.point.getX() - beg_x) * m_BedLevel.getGrid() / len;
            beg_y += (m_state.point.getY() - beg_y) * m_BedLevel.getGrid() / len;
            beg_z += (m_state.point.getZ() - beg_z) * m_BedLevel.getGrid() / len;
        }

        double correction;
        if (!m_BedLevel.get_z_correction(beg_x, beg_y, correction))
        {
            ERR_LOG();
            return false;
        }
        if (isXpresent) {
            par_loc['X'] = beg_x;
        }
        if (isYpresent) {
            par_loc['Y'] = beg_y;
        }
        par_loc['Z'] = beg_z + correction;
        if (!CGcodeProducer::produceCmd(cmd, par_loc))
        {
            return false;
        }
    } while (beg_x != m_state.point.getX() || beg_y != m_state.point.getY() || beg_z != m_state.point.getZ());

    return true;
}

bool CGcodeBedLevelProducer::produceCmd(const std::string &cmd, const tGCodeParams &par)
{
    bool result;
    if (cmd == "G0" || cmd == "G1")
    {
        result = produceLine(cmd, par);
    } else
    {
        result = CGcodeProducer::produceCmd(cmd, par);
    }
    m_pre_state = m_state;
    return result;
}

void CGcodeBedLevelProducer::clear()
{
    CGcodeParserSTD::clear();
    m_pre_state = { };
}
