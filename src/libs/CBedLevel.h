/*
 * CBedLevel.h
 *
 *  Created on: 27 лют. 2020 р.
 *      Author: ominenko
 */

#pragma once
#include <stdint.h>
#include <vector>

#ifdef UNIT_TEST
#include "gtest/gtest.h"
#endif

class CBedLevel {
    const uint16_t m_sizeX;
    const uint16_t m_sizeY;
    const uint16_t m_grid;
    const std::vector<double> m_ZheighArray;
    bool m_isInited;
    /**
     *                           z2   --|
     *                 z0        |      |
     *                  |        |      + (z2-z1)
     *   z1             |        |      |
     * ---+-------------+--------+--  --|
     *   a1            a0        a2
     *    |<---delta_a---------->|
     *
     *  calc_z0 is the basis for all the Mesh Based correction. It is used to
     *  find the expected Z Height at a position between two known Z-Height locations.
     *
     *  It is fairly expensive with its 4 floating point additions and 2 floating point
     *  multiplications.
     */
    constexpr static double calc_z0(const double &a0, const double &a1, const double &z1, const double &a2, const double &z2)
    {
        return z1 + (z2 - z1) * (a0 - a1) / (a2 - a1);
    }
    const double& mesh_z_value(uint16_t mX, uint16_t mY) const;
    uint16_t cell_index(const double &coord, const uint16_t max) const;
    public:
    uint16_t cell_index_x(const double &coord) const
    {
        return cell_index(coord, m_sizeX);
    }
    uint16_t cell_index_y(const double &coord) const
    {
        return cell_index(coord, m_sizeY);
    }
    CBedLevel(uint16_t sizeX, uint16_t sizeY, uint16_t grid, std::vector<double> ZheighArray);
    bool get_z_correction(const double &rx0, const double &ry0, double &correction) const;
    uint16_t getGrid() const
    {
        return m_grid;
    }
    bool isInitOk() const;
};


