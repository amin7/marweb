/*
 * CBedLevel_test.cpp
 *
 *  Created on: 27 лют. 2020 р.
 *      Author: ominenko
 */

#include "../src/libs/CBedLevel.h"
#include "gtest/gtest.h"

#include <string>

using namespace std;
//./obj/unittests.exe --gtest_filter=CBedLevelTest*
class CBedLevelTest: public testing::Test {
public:

};

TEST_F(CBedLevelTest, size)
{
    auto ZheighArray = std::vector<double>( { 0, 0, 0, 0, 0, 0, 0, 0, 0 });
    CBedLevel level(10, 10, 5, std::move(ZheighArray));
    double correction;
    EXPECT_TRUE(level.isInitOk());
    EXPECT_FALSE(level.get_z_correction(11, 10, correction));
    EXPECT_FALSE(level.get_z_correction(10, 11, correction));
    EXPECT_FALSE(level.get_z_correction(-1, 0, correction));
    EXPECT_FALSE(level.get_z_correction(0, -1, correction));

    EXPECT_TRUE(level.get_z_correction(0, 0, correction));
    EXPECT_TRUE(level.get_z_correction(10, 10, correction));
}

TEST_F(CBedLevelTest, sizeNOk)
{
    auto ZheighArray = std::vector<double>( { 0, 0, 0, 0, 0, 0, 0, 0 });
    CBedLevel level(10, 10, 5, std::move(ZheighArray));
    double correction;
    EXPECT_FALSE(level.isInitOk());
}

TEST_F(CBedLevelTest, dot)
{
    auto ZheighArray = std::vector<double>(
            { 0, 1, 3,
                    0, 2, 5,
                    0, 0, 4 });
    CBedLevel level(10, 10, 5, std::move(ZheighArray));
    double correction;
    EXPECT_TRUE(level.get_z_correction(0, 0, correction));
    EXPECT_EQ(correction, 0);
    EXPECT_TRUE(level.get_z_correction(5, 0, correction));
    EXPECT_EQ(correction, 1);
    EXPECT_TRUE(level.get_z_correction(5, 5, correction));
    EXPECT_EQ(correction, 2);
    EXPECT_TRUE(level.get_z_correction(10, 10, correction));
    EXPECT_EQ(correction, 4);

}

TEST_F(CBedLevelTest, line)
{
    auto ZheighArray = std::vector<double>(
            { 0, 1, 3,
                    0, 2, 5,
                    0, 0, 4 });
    CBedLevel level(10, 10, 5, std::move(ZheighArray));
    double correction;
    //X
    EXPECT_TRUE(level.get_z_correction(0, 1, correction));
    EXPECT_EQ(correction, 0);
    EXPECT_TRUE(level.get_z_correction(5, 2.5, correction));
    EXPECT_EQ(correction, 1.5);
    EXPECT_TRUE(level.get_z_correction(10, 7.5, correction));
    EXPECT_EQ(correction, 4.5);

    EXPECT_TRUE(level.get_z_correction(1, 0, correction));
    EXPECT_EQ(correction, 0.2);
    EXPECT_TRUE(level.get_z_correction(2.5, 5, correction));
    EXPECT_EQ(correction, 1);
    EXPECT_TRUE(level.get_z_correction(7.5, 10, correction));
    EXPECT_EQ(correction, 2);
}

TEST_F(CBedLevelTest, other)
{
    auto ZheighArray = std::vector<double>(
            { 0, 1, 3,
                    0, 2, 5,
                    0, 0, 4 });
    CBedLevel level(10, 10, 5, std::move(ZheighArray));
    double correction;
    //X
    EXPECT_TRUE(level.get_z_correction(2.5, 2.5, correction));
    EXPECT_DOUBLE_EQ(correction, 0.75);
    EXPECT_TRUE(level.get_z_correction(7.5, 2.5, correction));
    EXPECT_DOUBLE_EQ(correction, 2.75);
    EXPECT_TRUE(level.get_z_correction(9, 9, correction));
    EXPECT_DOUBLE_EQ(correction, 3.44);
}

TEST_F(CBedLevelTest, otherM)
{
    auto ZheighArray = std::vector<double>(
            { 0, -0.1, -0.3,
                    0, -0.2, -0.5,
                    0, 0, -0.4 });
    CBedLevel level(10, 10, 5, std::move(ZheighArray));
    double correction;
    //X
    EXPECT_TRUE(level.get_z_correction(2.5, 2.5, correction));
    EXPECT_DOUBLE_EQ(correction, -0.075);
    EXPECT_TRUE(level.get_z_correction(7.5, 2.5, correction));
    EXPECT_DOUBLE_EQ(correction, -0.275);
    EXPECT_TRUE(level.get_z_correction(9, 9, correction));
    EXPECT_DOUBLE_EQ(correction, -0.344);
}
