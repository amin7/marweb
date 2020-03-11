/*
 * CBedLevel.cpp
 *
 *  Created on: 27 лют. 2020 р.
 *      Author: ominenko
 */

#include "CBedLevel.h"
#include <math.h>
#include "logs.h"
CBedLevel::CBedLevel(uint16_t sizeX, uint16_t sizeY, uint16_t grid, std::vector<double> ZheighArray) :
        m_sizeX(sizeX), m_sizeY(sizeY), m_grid(grid), m_ZheighArray(std::move(ZheighArray))
{
    const auto arraySZ = (m_sizeX / m_grid + 1) * (m_sizeY / m_grid + 1);
    m_isInited = (m_ZheighArray.size() == arraySZ);
}

const double& CBedLevel::mesh_z_value(uint16_t mX, uint16_t mY) const
{
    return m_ZheighArray.at(mX + (m_sizeX / m_grid + 1) * mY);
}

bool CBedLevel::isInitOk() const
{
    return m_isInited;
}

uint16_t CBedLevel::cell_index(const double &coord, const uint16_t max) const
{
    if (coord <= 0)
    {
        return 0;
    }
    if (coord >= max)
    {
        return max / m_grid;
    }
    return coord / m_grid;
}

bool CBedLevel::get_z_correction(const double &rx0, const double &ry0, double &correction) const
{
    if (isInitOk() == false || 0 > rx0 || 0 > ry0 || m_sizeX < rx0 || m_sizeY < ry0)
    {
        return false;
    }

    const auto cx = cell_index(rx0, m_sizeX);
    const auto cy = cell_index(ry0, m_sizeY);

    const auto mx = cx * m_grid;
    const auto my = cy * m_grid;

    if (mx == rx0 && my == ry0)
    { //in dot
        correction = mesh_z_value(cx, cy);
        return true;
    }
    if (mx == rx0)
    { //in line X
        correction = calc_z0(ry0,
                my, mesh_z_value(cx, cy),
                my + m_grid, mesh_z_value(cx, cy + 1));
        return true;
    }

    if (my == ry0)
    { //in line Y
        correction = calc_z0(rx0,
                mx, mesh_z_value(cx, cy),
                mx + m_grid, mesh_z_value(cx + 1, cy));
        return true;
    }

    const auto z1 = calc_z0(rx0,
            mx, mesh_z_value(cx, cy),
            mx + m_grid, mesh_z_value(cx + 1, cy));

    const auto z2 = calc_z0(rx0,
            mx, mesh_z_value(cx, cy + 1),
            mx + m_grid, mesh_z_value(cx + 1, cy + 1));

    correction = calc_z0(ry0,
            my, z1,
            my + m_grid, z2);


    if (isnan(correction))
    {
        // if part of the Mesh is undefined, it will show up as NAN
        // in ubl.z_values[][] and propagate through the
        // calculations. If our correction is NAN, we throw it out
        // because part of the Mesh is undefined and we don't have the
        // information we need to complete the height correction.
        DBG_PRINTLN("isnan(correction)");
        return false;

    }
    return true;
}
