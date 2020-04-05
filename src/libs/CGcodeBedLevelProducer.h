/*
 * CGcodeBedLevelProducer.h
 *
 *  Created on: 2 бер. 2020 р.
 *      Author: ominenko
 */

#pragma once
#include <stdint.h>
#include <set>
#include <string>
#include "CGcodeParser.h"
#include "CBedLevel.h"
#include "CProbeArea.h"
#include "misk.h"

class CGcodeBedLevelProducer: public CGcodeProducer {
    CBedLevel m_BedLevel;
    tControlState m_pre_state;
    bool produceLineZmove(const std::string &cmd, const tGCodeParams &par);
    bool produceLine(const std::string &cmd, const tGCodeParams &par);
    public:
    static constexpr auto m_Ztolerance = 0.01; //if less - no z correction
    CGcodeBedLevelProducer(CGcodeParser &parcer, std::function<bool(const std::string &str)> writefn,
            const CProbeAreaGenerator &AreaGenerator);
    CGcodeBedLevelProducer(CGcodeParser &parcer, std::function<bool(const std::string &str)> writefn,
            CBedLevel &&BedLevel);
    bool produceCmd(const std::string &cmd, const tGCodeParams &par) override;
    void clear();
};
