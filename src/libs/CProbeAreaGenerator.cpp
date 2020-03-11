/*
 * CProbeAreaGenerator.cpp
 *
 *  Created on: 22 ñ³÷. 2020 ð.
 *      Author: ominenko
 */

#include "CProbeAreaGenerator.h"
/* y1      x2
 *
 *
 *         y3
 * x0
 */
bool CProbeAreaGenerator_line::getNext(int16_t &dX, int16_t &dY)
{
    if (0 == m_count)
    {
        return false;
    }
    m_count--;
    dX = m_stepX;
    dY = m_stepY;
    m_X += m_stepX;
    m_Y += m_stepY;

    if ((0 < m_stepX) && (m_X >= m_X0))
    {
        m_stepX = 0;
        m_stepY = m_step;
        m_X0 -= m_step;
        return true;
    }

    if ((0 < m_stepY) && (m_Y >= m_Y1))
    {
        m_stepX = -m_step;
        m_stepY = 0;
        m_Y1 -= m_step;
        return true;
    }

    if ((0 > m_stepX) && (m_X <= m_X2))
    {
        m_stepX = 0;
        m_stepY = -m_step;
        m_X2 += m_step;
        return true;
    }

    if ((0 > m_stepY) && (m_Y <= m_Y3))
    {
        m_stepX = m_step;
        m_stepY = 0;
        m_Y3 += m_step;
        return true;
    }
    return true;
}

bool CProbeAreaGenerator_line::init(uint16_t szX, uint16_t szY, uint16_t step)
{
    if (szX % step || szY % step)
    {
        return false; //icorrect size
    }
    m_szX = szX;
    m_szY = szY;
    m_step = step;
    reset();
    return true;
}

void CProbeAreaGenerator_line::reset()
{
    m_X0 = m_szX;
    m_Y1 = m_szY;
    m_X2 = 0;
    m_Y3 = m_step;

    m_stepX = m_step;
    m_stepY = 0;
    m_X = 0;
    m_Y = 0;
    m_count = (m_szX / m_step + 1) * (m_szY / m_step + 1) - 1;
    m_total_count = m_count;
}
