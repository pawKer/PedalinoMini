#include <unity.h>

#include "PedalinoCoreLogic.h"

extern "C" {

void setUp(void)
{
}

void tearDown(void)
{
}

}

namespace {

const int kControlChange = 11;
const int kProgramChange = 12;
const int kAnyChannel = 17;
const int kValueAny = 0;
const int kValueExact = 1;

bool matches(int triggerType,
             int triggerChannel,
             int triggerNumber,
             int valueMode,
             int triggerValue,
             int midiType,
             int channel,
             int data1,
             int data2)
{
  return pedalino::incoming_trigger_matches(triggerType,
                                            triggerChannel,
                                            triggerNumber,
                                            valueMode,
                                            triggerValue,
                                            midiType,
                                            channel,
                                            data1,
                                            data2,
                                            kControlChange,
                                            kProgramChange,
                                            kAnyChannel,
                                            kValueExact);
}

void test_map2_preserves_endpoints_and_rounds_midpoints()
{
  TEST_ASSERT_EQUAL_INT(0, pedalino::map2(0, 0, 10, 0, 100));
  TEST_ASSERT_EQUAL_INT(100, pedalino::map2(10, 0, 10, 0, 100));
  TEST_ASSERT_EQUAL_INT(50, pedalino::map2(5, 0, 10, 0, 100));
  TEST_ASSERT_EQUAL_INT(3, pedalino::map2(1, 0, 3, 0, 10));
  TEST_ASSERT_EQUAL_INT(7, pedalino::map2(2, 0, 3, 0, 10));
}

void test_map2_handles_zero_width_input_range()
{
  TEST_ASSERT_EQUAL_INT(0, pedalino::map2(10, 10, 10, 0, 100));
  TEST_ASSERT_EQUAL_INT(100, pedalino::map2(11, 10, 10, 0, 100));
}

void test_incoming_trigger_matches_any_channel_control_change()
{
  TEST_ASSERT_TRUE(matches(kControlChange,
                           kAnyChannel,
                           64,
                           kValueAny,
                           0,
                           kControlChange,
                           4,
                           64,
                           127));
}

void test_incoming_trigger_rejects_wrong_channel_or_number()
{
  TEST_ASSERT_FALSE(matches(kControlChange,
                            3,
                            64,
                            kValueAny,
                            0,
                            kControlChange,
                            4,
                            64,
                            127));
  TEST_ASSERT_FALSE(matches(kControlChange,
                            kAnyChannel,
                            64,
                            kValueAny,
                            0,
                            kControlChange,
                            4,
                            65,
                            127));
}

void test_incoming_trigger_requires_exact_value_for_control_change()
{
  TEST_ASSERT_TRUE(matches(kControlChange,
                           kAnyChannel,
                           64,
                           kValueExact,
                           42,
                           kControlChange,
                           4,
                           64,
                           42));
  TEST_ASSERT_FALSE(matches(kControlChange,
                            kAnyChannel,
                            64,
                            kValueExact,
                            42,
                            kControlChange,
                            4,
                            64,
                            43));
}

void test_incoming_trigger_program_change_ignores_value_filter()
{
  TEST_ASSERT_TRUE(matches(kProgramChange,
                           kAnyChannel,
                           10,
                           kValueExact,
                           42,
                           kProgramChange,
                           2,
                           10,
                           0));
}

void test_incoming_trigger_rejects_unsupported_message_type()
{
  TEST_ASSERT_FALSE(matches(kControlChange,
                            kAnyChannel,
                            64,
                            kValueAny,
                            0,
                            99,
                            4,
                            64,
                            127));
}

} // namespace

int main(int argc, char** argv)
{
  (void)argc;
  (void)argv;

  UNITY_BEGIN();
  RUN_TEST(test_map2_preserves_endpoints_and_rounds_midpoints);
  RUN_TEST(test_map2_handles_zero_width_input_range);
  RUN_TEST(test_incoming_trigger_matches_any_channel_control_change);
  RUN_TEST(test_incoming_trigger_rejects_wrong_channel_or_number);
  RUN_TEST(test_incoming_trigger_requires_exact_value_for_control_change);
  RUN_TEST(test_incoming_trigger_program_change_ignores_value_filter);
  RUN_TEST(test_incoming_trigger_rejects_unsupported_message_type);
  return UNITY_END();
}
