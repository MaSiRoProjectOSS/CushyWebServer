/**
 * @file test_sample.cpp
 * @author Akari-mobility (masiro.to.akari@gmail.com)
 * @brief
 * @version 0.23.1
 * @date 2023-03-07
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */

#include <unity.h>

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

///////////////////////////////////////////////////////////////////

void test_setup(void)
{
    bool result = true;

    TEST_ASSERT_TRUE(result);
}

void test_loop(void)
{
    int result = 1;

    TEST_ASSERT_EQUAL(1, result);
}

///////////////////////////////////////////////////////////////////

void RUN_UNITY_TESTS()
{
    UNITY_BEGIN();
    //////////////////////////////////
    RUN_TEST(test_setup);
    RUN_TEST(test_loop);
    //////////////////////////////////
    UNITY_END();
}

///////////////////////////////////////////////////////////////////

#ifndef ARDUINO
#include <Arduino.h>
void setup()
{
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    RUN_UNITY_TESTS();
}
void loop()
{
}
#else
int main(int argc, char **argv)
{
    RUN_UNITY_TESTS();
    return 0;
}
#endif
