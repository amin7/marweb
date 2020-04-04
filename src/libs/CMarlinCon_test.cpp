/*
 * CGcodeParser_test.cpp
 *
 *  Created on: 19 груд. 2019 р.
 *      Author: ominenko
 */

#include "../src/libs/CMarlinCon.h"
#include "gtest/gtest.h"
#include "test_helper.h"

#include <string>

using namespace std;



class CMarlinConTest: public testing::Test {
public:

};

TEST_F(CMarlinConTest, basic)
{
    CMarlinCon_implTest MarlinCon;
    CMarlinCon_Listener_implTest Listener;
    MarlinCon.addListener(Listener);
    Listener.m_cmd = "cmd";
    MarlinCon.loop();
    EXPECT_EQ(MarlinCon.m_write, "cmd");
    EXPECT_EQ(Listener.getMbuff(), ">>cmd\n");
    MarlinCon.m_read = "rep";
    MarlinCon.loop();
    EXPECT_EQ(Listener.getMbuff(), ">>cmd\n");
    MarlinCon.m_read = "\n";
    MarlinCon.loop();
    EXPECT_EQ(Listener.getMbuff(), ">>cmd\nrep\n");

}

