/*
 * CGcodeBedLevelProducer_test.cpp
 *
 *  Created on: 2 бер. 2020 р.
 *      Author: ominenko
 */

//#define UNIT_TEST
#ifdef UNIT_TEST
#include "../src/libs/CGcodeBedLevelProducer.h"
#include "gtest/gtest.h"
#include <sstream>
#include <string>

using namespace std;
//./obj/unittests.exe --gtest_filter=CGcodeBedLevelProducerTest*
class CGcodeBedLevelProducerTest: public testing::Test {
public:
    ostringstream os_cmd;

};

TEST_F(CGcodeBedLevelProducerTest, one_axis)
{
    CGcodeParser parser;
    CGcodeBedLevelProducer producer(parser, [&](const std::string &str) -> bool
            {
                os_cmd<<str;
                return true;
            }, CBedLevel(15, 15, 5,
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }));
//X
    parser.addCmd("G1 X2 Y0");
    EXPECT_EQ(os_cmd.str(), "G1 X2 Y0 Z0\n");

    os_cmd = { };
    producer.clear();
    parser.addCmd("G1 X6 Y0");
    EXPECT_EQ(os_cmd.str(), "G1 X3 Y0 Z0\nG1 X6 Y0 Z0\n");

    os_cmd = { };
    producer.clear();
    parser.addCmd("G1 X15 Y0");
    EXPECT_EQ(os_cmd.str(), "G1 X5 Y0 Z0\nG1 X10 Y0 Z0\nG1 X15 Y0 Z0\n");
//Y
    os_cmd = { };
    producer.clear();
    parser.addCmd("G1 X0 Y2");
    EXPECT_EQ(os_cmd.str(), "G1 X0 Y2 Z0\n");

    os_cmd = { };
    producer.clear();
    parser.addCmd("G1 X0 Y6");
    EXPECT_EQ(os_cmd.str(), "G1 X0 Y3 Z0\nG1 X0 Y6 Z0\n");

    os_cmd = { };
    producer.clear();
    parser.addCmd("G1 X0 Y15");
    EXPECT_EQ(os_cmd.str(), "G1 X0 Y5 Z0\nG1 X0 Y10 Z0\nG1 X0 Y15 Z0\n");

    //Z
    os_cmd = { };
    producer.clear();
    parser.addCmd("G1 X0 Y0 Z2");
    EXPECT_EQ(os_cmd.str(), "G1 X0 Y0 Z2\n");

    os_cmd = { };
    producer.clear();
    parser.addCmd("G1 X0 Y0 Z10");
    EXPECT_EQ(os_cmd.str(), "G1 X0 Y0 Z10\n");

}

TEST_F(CGcodeBedLevelProducerTest, axis_xz)
{
    CGcodeParser parser;
    CGcodeBedLevelProducer producer(parser, [&](const std::string &str) -> bool
            {
                os_cmd<<str;
                return true;
            }, CBedLevel(15, 15, 5,
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }));
//X
    parser.addCmd("G1 X2 Y0 Z1");
    EXPECT_EQ(os_cmd.str(), "G1 X2 Y0 Z1\n");

    os_cmd = { };
    producer.clear();
    parser.addCmd("G1 X6 Y0 Z2");
    EXPECT_EQ(os_cmd.str(), "G1 X3 Y0 Z1\nG1 X6 Y0 Z2\n");

    os_cmd = { };
    producer.clear();
    parser.addCmd("G1 X15 Y0 Z3");
    EXPECT_EQ(os_cmd.str(), "G1 X5 Y0 Z1\nG1 X10 Y0 Z2\nG1 X15 Y0 Z3\n");
}

TEST_F(CGcodeBedLevelProducerTest, xy)
{
    CGcodeParser parser;
    CGcodeBedLevelProducer producer(parser, [&](const std::string &str) -> bool
            {
                os_cmd<<str;
                return true;
            }, CBedLevel(15, 15, 5,
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }));

    parser.addCmd("G1 X2 Y2");
    EXPECT_EQ(os_cmd.str(), "G1 X2 Y2 Z0\n");

    os_cmd = { };
    producer.clear();
    parser.addCmd("G1 X6 Y2");
    EXPECT_EQ(os_cmd.str(), "G1 X3 Y1 Z0\nG1 X6 Y2 Z0\n");

    os_cmd = { };
    producer.clear();
    parser.addCmd("G1 X15 Y15");
    EXPECT_EQ(os_cmd.str(), "G1 X3.536 Y3.536 Z0\nG1 X7.071 Y7.071 Z0\nG1 X10.607 Y10.607 Z0\nG1 X12.803 Y12.803 Z0\nG1 X15 Y15 Z0\n");
}

#endif
