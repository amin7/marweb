/*
 * CProbeArea.h
 *
 *  Created on: Jan 11, 2020
 *      Author: family
 */

#pragma once
#include <string>
#include <stdint.h>
#include <vector>
#include <functional>
#include <math.h>
#include "CProbeAreaGenerator.h"
#ifdef UNIT_TEST
#include "gtest/gtest.h"
#endif
#include "CMarlinConCMDListener.h"

class CProbeArea: public CMarlinRun {
    private:
    static constexpr auto end_gcode = "G91\nG0 F300 Z2";
    static constexpr auto end_msg_done = "M117 probing done";
    static constexpr auto end_msg_stop = "M117 probing stop";
    double m_levelDelta;
    uint16_t m_feedRateXY;
    uint16_t m_feedRateProbe;
    uint16_t m_sizeX;
    uint16_t m_sizeY;
    uint16_t m_grid;
    std::vector<double> m_ZheighArray;
    CProbeAreaGenerator_line m_AreaGenerator;
    bool positionCallback(std::string &result);
    static double getZ(const std::string &result);
public:
    uint16_t getSizeX() const
    {
        return m_sizeX;
    }
    uint16_t getSizeY() const
    {
        return m_sizeY;
    }
    uint16_t getGrid() const
    {
        return m_grid;
    }
    std::vector<double> getZheighArray() const;
    void stop();
    static bool isErrorResult(const std::string &result);
    bool run(uint16_t sizeX, uint16_t sizeY, uint16_t grid, double levelDelta, uint16_t feedRateXY, uint16_t feedRateProbe, bool doubleTouch);
#ifdef UNIT_TEST
    FRIEND_TEST(CProbeAreaTest,elements);
    FRIEND_TEST(CProbeAreaTest,ok);
#endif
};

