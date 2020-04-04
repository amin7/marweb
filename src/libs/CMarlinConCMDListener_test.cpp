/*
 * CGcodeParser_test.cpp
 *
 *  Created on: 19 груд. 2019 р.
 *      Author: ominenko
 */
#include "../src/libs/CMarlinConCMDListener.h"
#include "gtest/gtest.h"

#include <string>

using namespace std;

class CMarlinCMD_ListenerTest: public testing::Test {
public:
    string m_buff;
    string m_buff1;
    bool tstHandler(string &buff)
    {
        m_buff = buff;
        return true;
    }
    ;
    bool tstHandler1(string &buff)
    {
        m_buff1 = buff;
        return true;
    }
    ;

};

TEST_F(CMarlinCMD_ListenerTest, cmdQueue)
{
    CMarlinConCMD_Listener CMD;
    EXPECT_EQ(CMD.getCmd(),"");

    CMD.addCmd("G28");
    EXPECT_EQ(CMD.getCmd(),"G28");

    CMD.addCmd("G28 as");

    EXPECT_TRUE(CMD.getCmd() == "");

    CMD.pushLine("X:10");
    EXPECT_TRUE(CMD.getCmd() == "");

    CMD.pushLine("ok T:26.4 /100.0 @0 B:24.9 /0.0 @0");

    EXPECT_EQ(CMD.getCmd(), "G28 as");

    EXPECT_EQ(CMD.getCmd(), "");
    CMD.pushLine("ok P15 B3");
    EXPECT_EQ(CMD.getCmd(), "");
}

TEST_F(CMarlinCMD_ListenerTest, cmdHandler)
{
    CMarlinConCMD_Listener CMD;
    m_buff = "";
    CMD.addCmd("G1 X10", bind(CMarlinCMD_ListenerTest::tstHandler, this, placeholders::_1));
    CMD.getCmd();
    EXPECT_EQ(m_buff, "");
    CMD.pushLine("ok P15 B3");
    EXPECT_EQ(m_buff, "ok P15 B3\n");

    m_buff = "1";
    CMD.pushLine("15 B3");
    EXPECT_EQ(m_buff, "1");
    CMD.pushLine("15 B312");
    CMD.pushLine("ok P15 B3");
    EXPECT_EQ(m_buff, "1");
    CMD.addCmd("G1 X10", bind(CMarlinCMD_ListenerTest::tstHandler, this, placeholders::_1));
    CMD.getCmd();
    CMD.pushLine("111");
    CMD.pushLine("2222");
    EXPECT_EQ(m_buff, "1");
    CMD.pushLine("ok");
    EXPECT_EQ(m_buff, "111\n2222\nok\n");
}

TEST_F(CMarlinCMD_ListenerTest, cmdHandler2)
{
    CMarlinConCMD_Listener CMD;
    m_buff = "";
    m_buff1 = "";
    CMD.addCmd("G1 X10", bind(CMarlinCMD_ListenerTest::tstHandler, this, placeholders::_1));
    CMD.addCmd("G2 X10 Y10", bind(CMarlinCMD_ListenerTest::tstHandler1, this, placeholders::_1));
    CMD.getCmd();
    CMD.pushLine("ok 1");
    EXPECT_EQ(m_buff, "ok 1\n");
    EXPECT_EQ(m_buff1, "");
    CMD.getCmd();
    CMD.pushLine("ok 2");
    EXPECT_EQ(m_buff, "ok 1\n");
    EXPECT_EQ(m_buff1, "ok 2\n");
}

TEST_F(CMarlinCMD_ListenerTest, fewCmd)
{
    CMarlinConCMD_Listener CMD;
    CMD.addCmd("G1 X10\nG38.2 F11\nM114");
    EXPECT_EQ(CMD.getCmd(),"G1 X10");
    CMD.pushLine("ok");
    EXPECT_EQ(CMD.getCmd(),"G38.2 F11");
    CMD.pushLine("ok");
    EXPECT_EQ(CMD.getCmd(),"M114");
    CMD.pushLine("ok");
}

