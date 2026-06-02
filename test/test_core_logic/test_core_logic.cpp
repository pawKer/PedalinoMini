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
const int kLinear = 0;
const int kLog = 1;
const int kAntilog = 2;
const int kPress = 1;
const int kRelease = 2;
const int kSlotUnchanged = -1;
const int kSlotOff = 0;
const int kSlotOn = 1;
const int kRgb = 0;
const int kRbg = 1;
const int kGrb = 2;
const int kGbr = 3;
const int kBrg = 4;
const int kBgr = 5;

struct TestAction {
  int targetAction;
  int bank;
};

struct TestTrigger {
  int id;
  int triggerType;
  int channel;
  int number;
  int valueMode;
  int value;
  int actionCount;
  TestAction actions[3];
};

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

void test_incoming_trigger_rejects_trigger_type_mismatch()
{
  TEST_ASSERT_FALSE(matches(kProgramChange,
                            kAnyChannel,
                            64,
                            kValueAny,
                            0,
                            kControlChange,
                            4,
                            64,
                            127));
}

void test_run_matching_incoming_triggers_preserves_group_order()
{
  TestTrigger triggers[] = {
    {1, kControlChange, kAnyChannel, 64, kValueAny, 0, 1, {{100, 0}, {0, 0}, {0, 0}}},
    {2, kControlChange, kAnyChannel, 65, kValueAny, 0, 1, {{200, 0}, {0, 0}, {0, 0}}},
    {3, kControlChange, kAnyChannel, 64, kValueAny, 0, 1, {{300, 0}, {0, 0}, {0, 0}}},
  };
  int matched[2] = {0, 0};
  int count = 0;

  pedalino::run_matching_incoming_triggers(triggers,
                                           3,
                                           kControlChange,
                                           4,
                                           64,
                                           127,
                                           kControlChange,
                                           kProgramChange,
                                           kAnyChannel,
                                           kValueExact,
                                           [&](const TestTrigger& trigger) {
                                             matched[count++] = trigger.id;
                                           });

  TEST_ASSERT_EQUAL_INT(2, count);
  TEST_ASSERT_EQUAL_INT(1, matched[0]);
  TEST_ASSERT_EQUAL_INT(3, matched[1]);
}

void test_incoming_bank_scopes_run_global_then_current_bank_after_handoff()
{
  int currentBank = 2;
  int visited[2] = {-1, -1};
  int count = 0;

  pedalino::run_incoming_bank_scopes(currentBank, [&](int bank) {
    visited[count++] = bank;
    if (bank == 0) currentBank = 5;
  });

  TEST_ASSERT_EQUAL_INT(2, count);
  TEST_ASSERT_EQUAL_INT(0, visited[0]);
  TEST_ASSERT_EQUAL_INT(5, visited[1]);
}

void test_incoming_bank_scopes_skip_second_pass_when_current_bank_is_global()
{
  int currentBank = 0;
  int count = 0;

  pedalino::run_incoming_bank_scopes(currentBank, [&](int bank) {
    TEST_ASSERT_EQUAL_INT(0, bank);
    count++;
  });

  TEST_ASSERT_EQUAL_INT(1, count);
}

void test_map_analog_clamps_and_maps_linear_response()
{
  TEST_ASSERT_EQUAL_INT(0, pedalino::map_analog_value(50, 100, 900, 1000, kLinear, kLinear, kLog, kAntilog));
  TEST_ASSERT_EQUAL_INT(500, pedalino::map_analog_value(500, 100, 900, 1000, kLinear, kLinear, kLog, kAntilog));
  TEST_ASSERT_EQUAL_INT(999, pedalino::map_analog_value(950, 100, 900, 1000, kLinear, kLinear, kLog, kAntilog));
}

void test_map_analog_log_and_antilog_shape_midrange_values()
{
  const unsigned int linear = pedalino::map_analog_value(500, 100, 900, 1000, kLinear, kLinear, kLog, kAntilog);
  const unsigned int logValue = pedalino::map_analog_value(500, 100, 900, 1000, kLog, kLinear, kLog, kAntilog);
  const unsigned int antilogValue = pedalino::map_analog_value(500, 100, 900, 1000, kAntilog, kLinear, kLog, kAntilog);

  TEST_ASSERT_TRUE(logValue > linear);
  TEST_ASSERT_TRUE(antilogValue < linear);
  TEST_ASSERT_EQUAL_INT(0, pedalino::map_analog_value(100, 100, 900, 1000, kLog, kLinear, kLog, kAntilog));
  TEST_ASSERT_EQUAL_INT(999, pedalino::map_analog_value(900, 100, 900, 1000, kAntilog, kLinear, kLog, kAntilog));
}

void test_map_analog_preserves_zero_width_and_inverted_calibration_behavior()
{
  TEST_ASSERT_EQUAL_INT(0, pedalino::map_analog_value(100, 100, 100, 1000, kLinear, kLinear, kLog, kAntilog));
  TEST_ASSERT_EQUAL_INT(0, pedalino::map_analog_value(500, 100, 100, 1000, kLinear, kLinear, kLog, kAntilog));

  TEST_ASSERT_EQUAL_INT(0, pedalino::map_analog_value(500, 900, 100, 1000, kLinear, kLinear, kLog, kAntilog));
  TEST_ASSERT_EQUAL_INT(999, pedalino::map_analog_value(950, 900, 100, 1000, kLinear, kLinear, kLog, kAntilog));
}

void test_preferred_action_overlay_label_prefers_event_specific_tags()
{
  TEST_ASSERT_EQUAL_STRING("OFF", pedalino::preferred_action_overlay_label("OFF", "ON", kRelease, kRelease));
  TEST_ASSERT_EQUAL_STRING("ON", pedalino::preferred_action_overlay_label("OFF", "ON", kPress, kRelease));
  TEST_ASSERT_EQUAL_STRING("ON", pedalino::preferred_action_overlay_label("", "ON", kRelease, kRelease));
  TEST_ASSERT_EQUAL_STRING("OFF", pedalino::preferred_action_overlay_label("OFF", "", kPress, kRelease));
  TEST_ASSERT_EQUAL_STRING("", pedalino::preferred_action_overlay_label("", "", kPress, kRelease));
}

void test_slot_state_from_tags_returns_state_change_or_unchanged()
{
  TEST_ASSERT_EQUAL_INT(kSlotOn, pedalino::slot_state_from_tags("REC", "STOP", "STOP", kSlotUnchanged, kSlotOff, kSlotOn));
  TEST_ASSERT_EQUAL_INT(kSlotOff, pedalino::slot_state_from_tags("REC", "STOP", "REC", kSlotUnchanged, kSlotOff, kSlotOn));
  TEST_ASSERT_EQUAL_INT(kSlotUnchanged, pedalino::slot_state_from_tags("REC", "STOP", "PLAY", kSlotUnchanged, kSlotOff, kSlotOn));
  TEST_ASSERT_EQUAL_INT(kSlotUnchanged, pedalino::slot_state_from_tags("", "STOP", "STOP", kSlotUnchanged, kSlotOff, kSlotOn));
}

void test_swap_rgb_order_supports_all_orderings()
{
  const pedalino::RgbColor color = {1, 2, 3};

  pedalino::RgbColor swapped = pedalino::swap_rgb_order(color, kRgb, kRgb, kRbg, kGrb, kGbr, kBrg, kBgr);
  TEST_ASSERT_EQUAL_INT(1, swapped.red);
  TEST_ASSERT_EQUAL_INT(2, swapped.green);
  TEST_ASSERT_EQUAL_INT(3, swapped.blue);

  swapped = pedalino::swap_rgb_order(color, kRbg, kRgb, kRbg, kGrb, kGbr, kBrg, kBgr);
  TEST_ASSERT_EQUAL_INT(1, swapped.red);
  TEST_ASSERT_EQUAL_INT(3, swapped.green);
  TEST_ASSERT_EQUAL_INT(2, swapped.blue);

  swapped = pedalino::swap_rgb_order(color, kGrb, kRgb, kRbg, kGrb, kGbr, kBrg, kBgr);
  TEST_ASSERT_EQUAL_INT(2, swapped.red);
  TEST_ASSERT_EQUAL_INT(1, swapped.green);
  TEST_ASSERT_EQUAL_INT(3, swapped.blue);

  swapped = pedalino::swap_rgb_order(color, kGbr, kRgb, kRbg, kGrb, kGbr, kBrg, kBgr);
  TEST_ASSERT_EQUAL_INT(2, swapped.red);
  TEST_ASSERT_EQUAL_INT(3, swapped.green);
  TEST_ASSERT_EQUAL_INT(1, swapped.blue);

  swapped = pedalino::swap_rgb_order(color, kBrg, kRgb, kRbg, kGrb, kGbr, kBrg, kBgr);
  TEST_ASSERT_EQUAL_INT(3, swapped.red);
  TEST_ASSERT_EQUAL_INT(1, swapped.green);
  TEST_ASSERT_EQUAL_INT(2, swapped.blue);

  swapped = pedalino::swap_rgb_order(color, kBgr, kRgb, kRbg, kGrb, kGbr, kBrg, kBgr);
  TEST_ASSERT_EQUAL_INT(3, swapped.red);
  TEST_ASSERT_EQUAL_INT(2, swapped.green);
  TEST_ASSERT_EQUAL_INT(1, swapped.blue);
}

void test_trim_page_decision_skips_prefix_or_finishes_chunk()
{
  pedalino::TrimPageDecision decision = pedalino::trim_page_decision(0, 10, 0, 5, false);
  TEST_ASSERT_FALSE(decision.clearPage);
  TEST_ASSERT_TRUE(decision.complete);
  TEST_ASSERT_EQUAL_INT(0, decision.removePrefix);
  TEST_ASSERT_EQUAL_INT(4, decision.trimFrom);
  TEST_ASSERT_EQUAL_INT(0, decision.nextSkipped);

  decision = pedalino::trim_page_decision(10, 10, 15, 5, false);
  TEST_ASSERT_FALSE(decision.clearPage);
  TEST_ASSERT_TRUE(decision.complete);
  TEST_ASSERT_EQUAL_INT(5, decision.removePrefix);
  TEST_ASSERT_EQUAL_INT(4, decision.trimFrom);
  TEST_ASSERT_EQUAL_INT(0, decision.nextSkipped);
}

void test_trim_page_decision_clears_when_start_is_after_current_content()
{
  const pedalino::TrimPageDecision decision = pedalino::trim_page_decision(0, 10, 12, 5, false);

  TEST_ASSERT_TRUE(decision.clearPage);
  TEST_ASSERT_FALSE(decision.complete);
  TEST_ASSERT_EQUAL_INT(10, decision.nextSkipped);
}

void test_trim_page_decision_lastcall_finishes_partial_page()
{
  const pedalino::TrimPageDecision decision = pedalino::trim_page_decision(0, 3, 0, 5, true);

  TEST_ASSERT_FALSE(decision.clearPage);
  TEST_ASSERT_TRUE(decision.complete);
  TEST_ASSERT_EQUAL_INT(4, decision.trimFrom);
}

void test_tap_tempo_tracker_averages_after_third_tap_and_resets_on_timeout()
{
  unsigned long lastTap = 0;
  unsigned char readingPos = 0;
  unsigned int readings[3] = {99, 99, 99};

  pedalino::tap_tempo_reset(lastTap, readingPos, readings);
  TEST_ASSERT_EQUAL_INT(0, pedalino::tap_tempo_tap(lastTap, readingPos, readings, 1000, 3000));
  TEST_ASSERT_EQUAL_INT(0, pedalino::tap_tempo_tap(lastTap, readingPos, readings, 1500, 3000));
  TEST_ASSERT_EQUAL_INT(121, pedalino::tap_tempo_tap(lastTap, readingPos, readings, 2000, 3000));
  TEST_ASSERT_EQUAL_INT(120, pedalino::tap_tempo_tap(lastTap, readingPos, readings, 2500, 3000));

  TEST_ASSERT_EQUAL_INT(0, pedalino::tap_tempo_tap(lastTap, readingPos, readings, 7000, 3000));
  TEST_ASSERT_EQUAL_INT(7000, lastTap);
  TEST_ASSERT_EQUAL_INT(1, readingPos);
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
  RUN_TEST(test_incoming_trigger_rejects_trigger_type_mismatch);
  RUN_TEST(test_run_matching_incoming_triggers_preserves_group_order);
  RUN_TEST(test_incoming_bank_scopes_run_global_then_current_bank_after_handoff);
  RUN_TEST(test_incoming_bank_scopes_skip_second_pass_when_current_bank_is_global);
  RUN_TEST(test_map_analog_clamps_and_maps_linear_response);
  RUN_TEST(test_map_analog_log_and_antilog_shape_midrange_values);
  RUN_TEST(test_map_analog_preserves_zero_width_and_inverted_calibration_behavior);
  RUN_TEST(test_preferred_action_overlay_label_prefers_event_specific_tags);
  RUN_TEST(test_slot_state_from_tags_returns_state_change_or_unchanged);
  RUN_TEST(test_swap_rgb_order_supports_all_orderings);
  RUN_TEST(test_trim_page_decision_skips_prefix_or_finishes_chunk);
  RUN_TEST(test_trim_page_decision_clears_when_start_is_after_current_content);
  RUN_TEST(test_trim_page_decision_lastcall_finishes_partial_page);
  RUN_TEST(test_tap_tempo_tracker_averages_after_third_tap_and_resets_on_timeout);
  return UNITY_END();
}
