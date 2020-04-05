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
        const CProbeAreaGenerator &AreaGenerator) :
CGcodeBedLevelProducer(parcer, writefn,
                CBedLevel(AreaGenerator.getSizeX(), AreaGenerator.getSizeY(), AreaGenerator.getGrid(), AreaGenerator.getZheighArray()))
{
}
bool CGcodeBedLevelProducer::produceLineZmove(const std::string &cmd, const tGCodeParams &par) {
    auto par_loc = par;
    double corr;
    if (!m_BedLevel.get_z_correction(m_state.point.getX(), m_state.point.getY(), corr))
            {
        ERR_LOG();
        return false;
    }
    par_loc['Z'] = m_state.point.getZ() + corr;
    return CGcodeProducer::produceCmd(cmd, par_loc);
}
bool CGcodeBedLevelProducer::produceLine(const std::string &cmd, const tGCodeParams &par)
{
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

    if (!isXpresent && !isYpresent) {
        return produceLineZmove(cmd, par);
    }

    auto par_loc = par;
    DBG_PRINT(m_state.point.getX());
    DBG_PRINT(" ");
    DBG_PRINT(m_state.point.getY());
    DBG_PRINT(" ");
    DBG_PRINT(m_state.point.getZ());
    DBG_PRINT(" ");
    DBG_PRINTLN(m_BedLevel.isInitOk());

    auto beg_x = m_pre_state.point.getX();
    auto beg_y = m_pre_state.point.getY();
    auto len = hypot(m_state.point.getX() - beg_x, m_state.point.getY() - beg_y);
    if (0 == len) { //move to same ccordinate
        return produceLineZmove(cmd, par);
    }

    double correct_initial;
    if (!m_BedLevel.get_z_correction(beg_x, beg_y, correct_initial)) {
        ERR_LOG();
        return false;
    }
    auto beg_z = m_pre_state.point.getZ() + correct_initial;

    const auto step_div = ceil(len / m_BedLevel.getGrid());
    const auto dx = (m_state.point.getX() - beg_x) / step_div;
    const auto dy = (m_state.point.getY() - beg_y) / step_div;
    const auto dz = (m_state.point.getZ() - beg_z) / step_div;
    auto f_done = false;
    while (!f_done)
    {
        if (len <= m_BedLevel.getGrid())
        { //last
            beg_x = m_state.point.getX();
            beg_y = m_state.point.getY();
            beg_z = m_state.point.getZ();
            f_done = true;
        } else
        { // step on grid
            beg_x += dx;
            beg_y += dy;
            beg_z += dz;
            len -= m_BedLevel.getGrid();
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
    };
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
