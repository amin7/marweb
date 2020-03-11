/*
 * CGcodeParser_test.cpp
 *
 *  Created on: 19 груд. 2019 р.
 *      Author: ominenko
 */
//#define UNIT_TEST
#ifdef UNIT_TEST
#include "../src/libs/CProbeArea.h"
#include "test_helper.h"
#include "gtest/gtest.h"

#include <string>

using namespace std;

class CProbeAreaTest: public testing::Test {
public:

};

TEST_F(CProbeAreaTest, elements)
{
    EXPECT_TRUE(CProbeArea::isErrorResult("Error\nok"));
    EXPECT_FALSE(CProbeArea::isErrorResult("X:12\nok"));
    EXPECT_TRUE(isnan(CProbeArea::getZ("X:12 Y:0 Z0\nok")));
    EXPECT_EQ(CProbeArea::getZ("X:12 Y:0 Z:1\nok"), 1);
    EXPECT_EQ(CProbeArea::getZ("X:12 Y:0 Z:1.01\nok"), 1.01);
}

TEST_F(CProbeAreaTest, runErr1)
{
    CProbeArea area;
    CMarlinCon_implTest con;
    con.addListener(area);

    ASSERT_EQ(area.getMode(), CProbeArea::paIdle);
    EXPECT_TRUE(area.run(10, 10, 5, 1, 200, 20, false));
    ASSERT_EQ(area.getMode(), CProbeArea::paRun);

    auto max_cmdcount = 100;
    while (area.getMode() == CMarlinRun::paRun && (max_cmdcount ))
    {
        con.m_read = "ok\n";
        con.loop();
        max_cmdcount--;
    }
    EXPECT_TRUE(max_cmdcount>0);
    EXPECT_EQ(area.getMode(), CMarlinRun::paError);
}

TEST_F(CProbeAreaTest, runErr2)
{
    CProbeArea area;
    CMarlinCon_implTest con;
    con.addListener(area);

    ASSERT_EQ(area.getMode(), CProbeArea::paIdle);
    EXPECT_TRUE(area.run(10, 10, 5, 1, 200, 20, false));
    ASSERT_EQ(area.getMode(), CProbeArea::paRun);

    auto max_cmdcount = 100;
    while (area.getMode() == CMarlinRun::paRun && (max_cmdcount))
    {
        con.m_read = "Error:dsfdsf\nok\n";
        con.loop();
        max_cmdcount--;
    }
    EXPECT_TRUE(max_cmdcount>0);
    EXPECT_EQ(area.getMode(), CMarlinRun::paError);
}

TEST_F(CProbeAreaTest, ok)
{
    CProbeArea area;
    CMarlinCon_implTest con;
    con.addListener(area);

    ASSERT_EQ(area.getMode(), CProbeArea::paIdle);
    EXPECT_TRUE(area.run(10, 10, 5, 1, 200, 20, false));
    ASSERT_EQ(area.getMode(), CProbeArea::paRun);

    auto max_cmdcount = 100;
    while (area.getMode() == CMarlinRun::paRun && (max_cmdcount > (-1)))
    {
        con.m_read = "X:10 Y:10 Z:1\nok\n";
        con.loop();
        max_cmdcount--;
    }
    EXPECT_TRUE(max_cmdcount>0);
    EXPECT_EQ(area.getMode(), CMarlinRun::paDone);
    EXPECT_EQ(area.m_ZheighArray.size(), 9);
    for (const auto val : area.m_ZheighArray)
    {
        EXPECT_EQ(val, 1);
    }

    EXPECT_EQ(area.getSizeX(), 10);
    EXPECT_EQ(area.getSizeY(), 10);
    EXPECT_EQ(area.getGrid(), 5);
    auto ZheighArray = area.getZheighArray();
    EXPECT_EQ(ZheighArray.size(), 9);
    for (const auto val : ZheighArray)
    {
        EXPECT_EQ(val, 1);
    }
}

#endif

