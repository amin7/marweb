/*
 * CProbeAreaGenerator.h
 *
 *  Created on: 22 ñ³÷. 2020 ð.
 *      Author: ominenko
 */

#pragma once
#include <stdint.h>

class CProbeAreaGenerator {
public:
    virtual bool getNext(int16_t &dX, int16_t &dY)=0;
    virtual void reset()=0;
};

class CProbeAreaGenerator_line: public CProbeAreaGenerator {
    uint16_t m_step;
    uint16_t m_szX;
    uint16_t m_szY;

    uint16_t m_X0;
    uint16_t m_Y1;
    uint16_t m_X2;
    uint16_t m_Y3;
    int16_t m_stepX;
    int16_t m_stepY;
    uint16_t m_X;
    uint16_t m_Y;
    uint16_t m_count;
    uint16_t m_total_count;
public:
    //0--100 %
    uint16_t getDone() const
    {
        return (m_total_count - m_count);
    }
    uint16_t getTotal() const
    {
        return m_total_count;
    }
    bool getNext(int16_t &dX, int16_t &dY) override;
    bool init(uint16_t szX, uint16_t szY, uint16_t step);
    void reset() override;
};

