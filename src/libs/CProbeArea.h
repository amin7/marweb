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
#include "gcode.h"

class CProbeArea: public CMarlinRun {
    private:
    static constexpr auto probe_first_distance = 0.5;
    static constexpr auto beg_gcode = "M150 B255 P255 R255 U255\nM121";
    static constexpr auto end_gcode = "G91\nG0 F300 Z2";
    static constexpr auto end_msg_done = "M150 P255 U255\nM117 probing done";
    static constexpr auto end_msg_stop = "M150 P255 R255\nM117 probing stop";
    double m_levelDelta;
    uint16_t m_feedRateXY;
    uint16_t m_feedRateProbe;
    CProbeAreaGenerator_line m_AreaGenerator;
    bool positionCallback(std::string &result);
    static double getZ(const std::string &result);
public:
    const CProbeAreaGenerator& getAreaGenerator() const
    {
        return m_AreaGenerator;
    }
    void stop();
    static bool isErrorResult(const std::string &result);
    bool run(uint16_t sizeX, uint16_t sizeY, uint16_t grid, double levelDelta, uint16_t feedRateXY, uint16_t feedRateProbe, bool doubleTouch);
#ifdef UNIT_TEST
    FRIEND_TEST(CProbeAreaTest,elements);
#endif
};

