/*
 * CGcodeParser_test.cpp
 *
 *  Created on: 19 груд. 2019 р.
 *      Author: ominenko
 */
//#define UNIT_TEST
#ifdef UNIT_TEST
#include "../src/libs/CGcodeParser.h"
#include "gtest/gtest.h"

#include <string>

using namespace std;
//./obj/unittests.exe --gtest_filter=CGcodeParserTest*
class CGcodeParserTest: public testing::Test {
public:
    class CGcodeParser_imp: public CGcodeParser
    {
    public:
        string m_cmd;
        tGCodeParams m_param;
        CGcodeParser_imp()
        {
            m_GCodeHandlers.clear();
            m_GCodeHandlers.push_back([&](const string &cmd, const tGCodeParams &params) -> bool
                    {
                        m_cmd=cmd;
                        m_param=params;
                        return true;
                    }
                    );
        }
    };
    class CGcodeProducer_imp: public CGcodeProducer {
    public:
        string line;
        CGcodeProducer_imp(CGcodeParser &parcer) :
                CGcodeProducer(parcer, [&](const std::string &str) -> bool
                {
                    line=str;
                    return true;
                })
        {
        }
    };
};

TEST_F(CGcodeParserTest, Point){
    auto Point1=CPoint(0);
    auto Point2=CPoint(nan(""));
    Point1+=Point2;
    EXPECT_EQ(Point1,CPoint(0,0,0));
    Point2.setX(10);
    Point1 += Point2;
    EXPECT_EQ(Point1, CPoint(10, 0, 0));

}

TEST_F(CGcodeParserTest, cmd)
{
    CGcodeParser_imp parser_imp;

    EXPECT_EQ(parser_imp.addCmd("G1 X\nG1 Y\n"), true);

    EXPECT_EQ(parser_imp.addCmd("  G90"), true);
    EXPECT_TRUE(parser_imp.m_cmd == "G90");

    EXPECT_EQ(parser_imp.addCmd("G1 X Y"), true);
    EXPECT_TRUE(parser_imp.m_cmd == "G1");

    EXPECT_EQ(parser_imp.addCmd("M38.2 X Y"), true);
    EXPECT_TRUE(parser_imp.m_cmd == "M38.2");

    parser_imp.m_cmd = "noCall";
    EXPECT_EQ(parser_imp.addCmd(";G1"), true);
    EXPECT_TRUE(parser_imp.m_cmd == "noCall");
}

TEST_F(CGcodeParserTest, paramCount)
{
    CGcodeParser_imp parser_imp;
    EXPECT_EQ(parser_imp.addCmd("G28"), true);
    ASSERT_EQ(parser_imp.m_param.empty(), true);

    EXPECT_EQ(parser_imp.addCmd("G1 X ;Y"), true);
    ASSERT_EQ(parser_imp.m_param.size(), 1);

    EXPECT_EQ(parser_imp.addCmd("G1 X Y"), true);
    ASSERT_EQ(parser_imp.m_param.size(), 2);

    EXPECT_EQ(parser_imp.addCmd("G1 X10 Y+20 Z-30"), true);
    ASSERT_EQ(parser_imp.m_param.size(), 3);

    EXPECT_EQ(parser_imp.addCmd("G1 X10.12 Y+20.12 Z-30.12"), true);
    ASSERT_EQ(parser_imp.m_param.size(), 3);

    EXPECT_EQ(parser_imp.addCmd("G1; X.12 Y+.13 Z-.14"), true);
    EXPECT_EQ(parser_imp.m_param.empty(), true);

}

TEST_F(CGcodeParserTest, paramValue)
{
    CGcodeParser_imp parser_imp;
    EXPECT_EQ(parser_imp.addCmd("G1 X ;Y"), true);
    EXPECT_EQ(parser_imp.m_param.size(), 1);
    ASSERT_EQ(isnan(parser_imp.m_param['X']), true);

    EXPECT_EQ(parser_imp.addCmd("G1 X Y"), true);
    EXPECT_EQ(parser_imp.m_param.size(), 2);
    ASSERT_EQ(isnan(parser_imp.m_param['X']), true);
    ASSERT_EQ(isnan(parser_imp.m_param['Y']), true);
    ASSERT_EQ(parser_imp.m_param.count('Z'), 0);

    EXPECT_EQ(parser_imp.addCmd("G1 X10 Y+20 Z-30"), true);
    EXPECT_EQ(parser_imp.m_param.size(), 3);
    ASSERT_EQ(parser_imp.m_param['X'], 10);
    ASSERT_EQ(parser_imp.m_param['Y'], 20);
    ASSERT_EQ(parser_imp.m_param['Z'], -30);

    EXPECT_EQ(parser_imp.addCmd("G1 X10.12 Y+20.12 Z-30.12"), true);
    EXPECT_EQ(parser_imp.m_param.size(), 3);
    ASSERT_EQ(parser_imp.m_param['X'], 10.12);
    ASSERT_EQ(parser_imp.m_param['Y'], 20.12);
    ASSERT_EQ(parser_imp.m_param['Z'], -30.12);

    EXPECT_EQ(parser_imp.addCmd("G1 X.12 Y+.13 Z-.14"), true);
    EXPECT_EQ(parser_imp.m_param.size(), 3);
    ASSERT_EQ(parser_imp.m_param['X'], 0.12);
    ASSERT_EQ(parser_imp.m_param['Y'], 0.13);
    ASSERT_EQ(parser_imp.m_param['Z'], -0.14);
}

TEST_F(CGcodeParserTest, info)
{
    CGcodeParser parser;
    CGcodeParserSTD parserSTD(parser);
    CGcodeInfo info(parser);

    stringstream ss;
    EXPECT_EQ(parser.addCmd(";commet"), true);
    EXPECT_EQ(parser.addCmd("G28"), true);
    EXPECT_EQ(parser.addCmd("G90"), true);    //absolute

    EXPECT_EQ(parser.addCmd("G1 X10 Y20 Z30"), true);

    EXPECT_EQ(info.getMin(), CPoint(0, 0, 0));
    EXPECT_EQ(info.getMax(), CPoint(10, 20, 30));

    EXPECT_EQ(parser.addCmd("G1 X10 Y20 Z30"), true);
    EXPECT_EQ(info.getMin(), CPoint(0, 0, 0));
    EXPECT_EQ(info.getMax(), CPoint(10, 20, 30));

    EXPECT_EQ(parser.addCmd("G91"), true);    //retative
    EXPECT_EQ(parser.addCmd("G1 X10 Y20 Z30"), true);
    EXPECT_EQ(info.getMin(), CPoint(0, 0, 0));
    EXPECT_EQ(info.getMax(), CPoint(20, 40, 60));

    EXPECT_EQ(parser.addCmd("G28"), true);
    EXPECT_EQ(parser.addCmd("G1 X-10 Y-20 Z-30"), true);
    EXPECT_EQ(info.getMin(), CPoint(-10, -20, -30));
    EXPECT_EQ(info.getMax(), CPoint(20, 40, 60));
}

TEST_F(CGcodeParserTest, normalizeGCode) {
    auto res=normalizeGCode("");
    EXPECT_TRUE(res.empty());
    res=normalizeGCode(" ");
    EXPECT_TRUE(res.empty());
    res=normalizeGCode(" G1 X2; qwqw\n\n;ererer\nG2");
    EXPECT_EQ(res.size(),2);
    EXPECT_EQ(res.at(0),"G1 X2");
    EXPECT_EQ(res.at(1),"G2");
}

TEST_F(CGcodeParserTest, Producer) {
    CGcodeParser_imp parser_imp;
    CGcodeProducer_imp producer(parser_imp);
    tGCodeParams par;
    par.clear();
    EXPECT_TRUE(producer.produceCmd("G1", par));
    EXPECT_EQ(producer.line, "G1\n");
    par['X'] = nan("");
    par['Y'] = 10;
    par['Z'] = 10.1;
    EXPECT_TRUE(producer.produceCmd("G1", par));
    EXPECT_EQ(producer.line, "G1 X Y10 Z10.1\n");
}

#endif

