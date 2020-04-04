/*
 * CProbeAreaGenerator.h
 *
 *  Created on: 22 ñ³÷. 2020 ð.
 *      Author: ominenko
 */

#pragma once
#include <stdint.h>
#include <vector>

class CProbeAreaGenerator {
protected:
    int16_t m_X = 0;
    int16_t m_Y = 0;
    uint16_t m_sizeX;
    uint16_t m_sizeY;
    uint16_t m_grid;
    std::vector<double> m_ZheighArray;
public:
    int16_t getX() const {
        return m_X;
    }
    int16_t getY() const {
        return m_Y;
    }
    int16_t getSizeX() const {
        return m_sizeX;
    }
    int16_t getSizeY() const {
        return m_sizeY;
    }
    int16_t getGrid() const {
        return m_grid;
    }
    std::vector<double> getZheighArray() const {
        return m_ZheighArray;
    }
    void setZheigh(double Zheight);
    bool init(uint16_t szX, uint16_t szY, uint16_t grid);
    virtual bool getNext(int16_t &dX, int16_t &dY)=0;
    virtual void reset()=0;
};

class CProbeAreaGenerator_line: public CProbeAreaGenerator {
    uint16_t m_X0;
    uint16_t m_Y1;
    uint16_t m_X2;
    uint16_t m_Y3;
    int16_t m_stepX;
    int16_t m_stepY;
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
    void reset() override;
};

