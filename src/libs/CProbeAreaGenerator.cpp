/*
 * CProbeAreaGenerator.cpp
 *
 *  Created on: 22 ñ³÷. 2020 ð.
 *      Author: ominenko
 */

#include "CProbeAreaGenerator.h"
#include "math.h"
bool CProbeAreaGenerator::init(uint16_t sizeX, uint16_t sizeY, uint16_t grid)
        {
    const auto rX = ((sizeX + grid - 1) / grid);
    const auto rY = ((sizeY + grid - 1) / grid);

    m_sizeX = rX * grid;
    m_sizeY = rY * grid;
    m_grid = grid;
    const auto count = (rX + 1) * (rY + 1);
    m_ZheighArray.reserve(count);
    m_ZheighArray.resize(count);
    for (auto &val : m_ZheighArray) {
        val = nan("");
    }
    reset();
    return true;
}

void CProbeAreaGenerator::setZheigh(double Zheight) {
    m_ZheighArray.at((getX() + getY() * (getSizeX() / getGrid() + 1)) / getGrid()) = Zheight;
}
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
        m_stepY = getGrid();
        m_X0 -= getGrid();
        return true;
    }

    if ((0 < m_stepY) && (m_Y >= m_Y1))
    {
        m_stepX = -getGrid();
        m_stepY = 0;
        m_Y1 -= getGrid();
        return true;
    }

    if ((0 > m_stepX) && (m_X <= m_X2))
    {
        m_stepX = 0;
        m_stepY = -getGrid();
        m_X2 += getGrid();
        return true;
    }

    if ((0 > m_stepY) && (m_Y <= m_Y3))
    {
        m_stepX = getGrid();
        m_stepY = 0;
        m_Y3 += getGrid();
        return true;
    }
    return true;
}

void CProbeAreaGenerator_line::reset()
{
    m_X0 = getSizeX();
    m_Y1 = getSizeY();
    m_X2 = 0;
    m_Y3 = getGrid();

    m_stepX = getGrid();
    m_stepY = 0;
    m_X = 0;
    m_Y = 0;
    m_count = (getSizeX() / getGrid() + 1) * (getSizeY() / getGrid() + 1) - 1;
    m_total_count = m_count;
}
