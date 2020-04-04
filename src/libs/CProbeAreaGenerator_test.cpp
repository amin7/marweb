/*
 * CProbeAreaGenerator_test.cpp
 *
 *  Created on: 22 ñ³÷. 2020 ð.
 *      Author: ominenko
 */

#include "../src/libs/CProbeAreaGenerator.h"
#include "test_helper.h"
#include "gtest/gtest.h"

#include <string>
#include <array>
#include <iostream>

using namespace std;
//./obj/unittests.exe --gtest_filter=CProbeAreaGeneratorTest*
template<std::size_t nX, std::size_t nY>
class CTestedArray {

    bool surface[nX + 1][nY + 1] = { false };
    const uint16_t m_szX;
    const uint16_t m_szY;
    const uint16_t m_step;
public:
    CTestedArray(uint16_t step) :
            m_szX(nX * step), m_szY(nY * step), m_step(step)
    {
    }
    ;
    bool set(uint16_t x, uint16_t y)
    {
        if (x % m_step || x > m_szX)
        {
            return false;
        }
        if (y % m_step || y > m_szY)
        {
            return false;
        }
        surface[x / m_step][y / m_step] = true;
        return true;
    }

    bool get(uint16_t x, uint16_t y)
    {
        if (x % m_step || x > m_szX)
        {
            return false;
        }
        if (y % m_step || y > m_szY)
        {
            return false;
        }

        return surface[x / m_step][y / m_step];
    }

    bool check()
    {
        for (auto x = 0; x <= nX; x++)
            for (auto y = 0; y <= nY; y++)
            {
                if (!surface[x][y])
                {
                    return false;
                }
            }
        return true;
    }
};


class CProbeAreaGeneratorTest: public testing::Test {
public:

};

TEST_F(CProbeAreaGeneratorTest, test)
{
    CTestedArray<10 / 5, 10 / 5> arr(5);
    ASSERT_FALSE(arr.set(1, 0));
    ASSERT_FALSE(arr.set(0, 1));
    ASSERT_FALSE(arr.set(11, 0));
    ASSERT_FALSE(arr.set(0, 11));

    ASSERT_FALSE(arr.check());

    ASSERT_FALSE(arr.get(0, 0));
    ASSERT_TRUE(arr.set(0, 0));
    ASSERT_TRUE(arr.get(0, 0));
    for (auto x = 0; x <= 10; x += 5)
    {
        for (auto y = 0; y <= 10; y += 5)
        {
            ASSERT_TRUE(arr.set(x, y));
        }
    }
    ASSERT_TRUE(arr.check());
}

TEST_F(CProbeAreaGeneratorTest, generator)
{
    constexpr auto szX = 10;
    constexpr auto szY = 20;
    constexpr auto step = 1;
    CTestedArray<szX / step, szY / step> arr(step);
    CProbeAreaGenerator_line Generator;
    ASSERT_TRUE(Generator.init(szX, szY,step));
    auto maxcount = (szX / step + 1) * (szY / step + 1);
    int16_t X = 0;
    int16_t Y = 0;
    int16_t dX;
    int16_t dY;
    arr.set(0, 0);
    while (maxcount && Generator.getNext(dX, dY))
    {
        maxcount--;
        X += dX;
        Y += dY;
        ASSERT_TRUE(Generator.getX() <= szX);
        ASSERT_TRUE(Generator.getY() <= szY);
        ASSERT_TRUE(Generator.getX() >= 0);
        ASSERT_TRUE(Generator.getY() >= 0);
        ASSERT_EQ(X, Generator.getX());
        ASSERT_EQ(Y, Generator.getY());
        ASSERT_FALSE(arr.get(X, Y)); //doubleset
        ASSERT_TRUE(arr.set(X, Y));
    }
    ASSERT_EQ(maxcount, 1);
    ASSERT_TRUE(arr.check());
}

