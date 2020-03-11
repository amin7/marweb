/*
 * CGcodeParser.h
 *
 *  Created on: 19 груд. 2019 р.
 *      Author: ominenko
 */

#pragma once

#include <map>
#include <vector>
#include <functional>
#include <string>
#include <string.h>
#include <math.h>
#include "misk.h"

class CPoint {
    double x;
    double y;
    double z;
    public:
    void copyNNN(const CPoint &rhs) //copy Not NaN
    {
        if (!isnan(rhs.x))
            this->x = rhs.x;
        if (!isnan(rhs.y))
            this->y = rhs.y;
        if (!isnan(rhs.z))
            this->z = rhs.z;
    }
    CPoint(double _x, double _y, double _z) :
            x(_x), y(_y), z(_z)
    {

    }
    CPoint(double _val) :
            x(_val), y(_val), z(_val)
    {

    }
    CPoint() :
            CPoint(0, 0, 0)
    {

    }
    void setX(double val)
    {
        x = val;
    }
    void setY(double val)
    {
        y = val;
    }
    void setZ(double val)
    {
        z = val;
    }
    const double& getX() const
    {
        return x;
    }
    const double& getY() const
    {
        return y;
    }
    const double& getZ() const
    {
        return z;
    }
    void operator+=(const CPoint &rhs)
    {
        if (!isnan(rhs.x))
            this->x += rhs.x;
        if (!isnan(rhs.y))
            this->y += rhs.y;
        if (!isnan(rhs.z))
            this->z += rhs.z;
    }
    CPoint operator+(const CPoint &rhs) const
    {
        CPoint result;
        if (!isnan(rhs.x))
            result.x = this->x + rhs.x;
        if (!isnan(rhs.y))
            result.y = this->y + rhs.y;
        if (!isnan(rhs.z))
            result.z = this->z + rhs.z;
        return result;
    }
    bool operator==(const CPoint &rhs) const
    {
        if (this->x != rhs.x)
            return false;
        if (this->y != rhs.y)
            return false;
        if (this->z != rhs.z)
            return false;
        return true;
    }
};

typedef struct {
    bool isAbsolutePositioning = true;
    CPoint point;
    double feed = 0;
}tControlState;

typedef std::map<char, double> tGCodeParams;
typedef std::function<bool(const std::string&, const tGCodeParams&)> tGCodeHandler;

std::vector<std::string> normalizeGCode(const std::string &in);

class CGcodeParser {
protected:
    std::vector<tGCodeHandler> m_GCodeHandlers;
    uint16_t lineTotal = 0;
    uint16_t lineProcessed = 0;
    public:
    void addHandler(tGCodeHandler handler);
    bool addCmd(const std::string &cmd);

    const auto getLineTotal() const
    {
        return lineTotal;
    }
    const auto getLineProcessed() const
    {
        return lineProcessed;
    }
    virtual ~CGcodeParser()
    {
    }
};

typedef std::function<bool(const tGCodeParams&, tControlState&)> tGCodeCmdHandler;
class CGcodeParserSTD {
    protected:
    tControlState m_state;
    std::map<const char*, tGCodeCmdHandler, CompareCStrings> m_cmdHandlers;
    public:
    void clear();
    CGcodeParserSTD(CGcodeParser &parcer);
};


class CGcodeInfo: public CGcodeParserSTD {
    CPoint max;
    CPoint min;
    public:
    CGcodeInfo(CGcodeParser &parcer);
    const CPoint& getMax() const
    {
        return max;
    }
    const CPoint& getMin() const
    {
        return min;
    }
};

class CGcodeProducer: public CGcodeParserSTD  {
    static constexpr auto precision_mul = 1000;
    std::function<bool(const std::string &str)> m_write;
    public:
    static constexpr auto print_comment = true;
    CGcodeProducer(CGcodeParser &parcer,std::function<bool(const std::string &str)>);
    virtual ~CGcodeProducer()
    {
    }
    bool add_comment(const std::string &comment);
    virtual bool produceCmd(const std::string &cmd, const tGCodeParams &par);
};
