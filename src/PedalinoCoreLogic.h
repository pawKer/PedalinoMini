#ifndef _PEDALINO_CORE_LOGIC_H
#define _PEDALINO_CORE_LOGIC_H

#include <cmath>
#include <cstring>

namespace pedalino {

struct RgbColor {
  int red;
  int green;
  int blue;
};

struct TrimPageDecision {
  bool clearPage;
  bool complete;
  unsigned int removePrefix;
  unsigned int trimFrom;
  unsigned int nextSkipped;
};

struct HardwareControlMapping {
  bool supported;
  unsigned char pedal;
  unsigned char button;
  const char* reason;
};

inline long map2(long x, long in_min, long in_max, long out_min, long out_max)
{
  const long dividend = out_max - out_min;
  const long divisor = in_max - in_min;
  const long delta = x - in_min;

  if (x == in_min) return out_min;
  if (x == in_max) return out_max;

  return (divisor == 0 ? (x <= in_min ? out_min : out_max) : (delta * dividend + (divisor / 2)) / divisor + out_min);
}

inline unsigned int clamp_unsigned(unsigned int value, unsigned int minValue, unsigned int maxValue)
{
  if (value < minValue) return minValue;
  if (value > maxValue) return maxValue;
  return value;
}

inline unsigned int map_analog_value(unsigned int value,
                                     unsigned int expZero,
                                     unsigned int expMax,
                                     unsigned int adcResolution,
                                     int analogResponse,
                                     int linearResponse,
                                     int logResponse,
                                     int antilogResponse)
{
  value = clamp_unsigned(value, expZero, expMax);
  value = (unsigned int)map2(value, expZero, expMax, 0, adcResolution - 1);

  if (analogResponse == logResponse) {
    value = (unsigned int)std::round((adcResolution - 1) * std::log1p(value) / std::log(adcResolution));
  } else if (analogResponse == antilogResponse) {
    value = (unsigned int)std::round((std::exp(3 * value / (double)(adcResolution - 1)) - 1) / (std::expm1(3)) * (adcResolution - 1));
  } else {
    (void)linearResponse;
  }

  return value;
}

inline bool incoming_trigger_matches(int triggerType,
                                     int triggerChannel,
                                     int triggerNumber,
                                     int valueMode,
                                     int triggerValue,
                                     int midiType,
                                     int channel,
                                     int data1,
                                     int data2,
                                     int controlChangeType,
                                     int programChangeType,
                                     int anyChannel,
                                     int exactValueMode)
{
  if (midiType != controlChangeType && midiType != programChangeType) return false;
  if (triggerType != midiType) return false;
  if (triggerChannel != anyChannel && triggerChannel != channel) return false;
  if (triggerNumber != data1) return false;
  if (midiType == controlChangeType && valueMode == exactValueMode && triggerValue != data2) return false;

  return true;
}

template <typename Trigger, typename Callback>
inline void run_matching_incoming_triggers(const Trigger* triggers,
                                           int triggerCount,
                                           int midiType,
                                           int channel,
                                           int data1,
                                           int data2,
                                           int controlChangeType,
                                           int programChangeType,
                                           int anyChannel,
                                           int exactValueMode,
                                           Callback onMatch)
{
  if (triggers == 0) return;

  for (int i = 0; i < triggerCount; i++) {
    const Trigger& trigger = triggers[i];
    if (incoming_trigger_matches(trigger.triggerType,
                                 trigger.channel,
                                 trigger.number,
                                 trigger.valueMode,
                                 trigger.value,
                                 midiType,
                                 channel,
                                 data1,
                                 data2,
                                 controlChangeType,
                                 programChangeType,
                                 anyChannel,
                                 exactValueMode)) {
      onMatch(trigger);
    }
  }
}

template <typename Bank, typename RunBank>
inline void run_incoming_bank_scopes(Bank& currentBank, RunBank runBank)
{
  runBank(0);
  if (currentBank != 0) runBank(currentBank);
}

inline const char* preferred_action_overlay_label(const char* tagOff,
                                                  const char* tagOn,
                                                  int event,
                                                  int releaseEvent)
{
  const bool hasTagOff = tagOff != 0 && tagOff[0] != 0;
  const bool hasTagOn  = tagOn != 0 && tagOn[0] != 0;

  if (hasTagOff && hasTagOn) {
    return event == releaseEvent ? tagOff : tagOn;
  }

  if (hasTagOn)  return tagOn;
  if (hasTagOff) return tagOff;

  return "";
}

inline int slot_state_from_tags(const char* tagOff,
                                const char* tagOn,
                                const char* lastLabel,
                                int unchanged,
                                int offState,
                                int onState)
{
  if (tagOff == 0 || tagOn == 0 || lastLabel == 0) return unchanged;
  if (tagOff[0] == 0 || tagOn[0] == 0) return unchanged;

  if (std::strcmp(lastLabel, tagOn) == 0) return onState;
  if (std::strcmp(lastLabel, tagOff) == 0) return offState;

  return unchanged;
}

inline RgbColor swap_rgb_order(RgbColor color,
                               int order,
                               int rgbOrder,
                               int rbgOrder,
                               int grbOrder,
                               int gbrOrder,
                               int brgOrder,
                               int bgrOrder)
{
  if (order == rgbOrder) return color;
  if (order == rbgOrder) return {color.red, color.blue, color.green};
  if (order == grbOrder) return {color.green, color.red, color.blue};
  if (order == gbrOrder) return {color.green, color.blue, color.red};
  if (order == brgOrder) return {color.blue, color.red, color.green};
  if (order == bgrOrder) return {color.blue, color.green, color.red};

  return {0, 0, 0};
}

inline TrimPageDecision trim_page_decision(unsigned int skipped,
                                           unsigned int saved,
                                           unsigned int start,
                                           unsigned int len,
                                           bool lastcall)
{
  const unsigned int fullPageLength = skipped + saved;
  TrimPageDecision decision = {false, false, 0, len - 1, skipped};

  if (start > (fullPageLength - 1)) {
    decision.clearPage = true;
    decision.nextSkipped = skipped + saved;
    return decision;
  }

  if (start > (fullPageLength - saved)) {
    decision.removePrefix = start - (fullPageLength - saved);
    decision.nextSkipped = skipped + decision.removePrefix;
  }

  if (fullPageLength >= (start + len) || lastcall) {
    decision.complete = true;
    decision.nextSkipped = 0;
  }

  return decision;
}

inline bool hardware_pedal_mode_is_virtual_pressable(int pedalMode,
                                                     int momentary1Mode,
                                                     int momentary2Mode,
                                                     int momentary3Mode,
                                                     int ladderMode,
                                                     int analogMomentaryMode)
{
  return pedalMode == momentary1Mode ||
         pedalMode == momentary2Mode ||
         pedalMode == momentary3Mode ||
         pedalMode == ladderMode ||
         pedalMode == analogMomentaryMode;
}

inline HardwareControlMapping hardware_control_mapping_for(int pedal1,
                                                           int button1,
                                                           int pedal2,
                                                           int button2,
                                                           int pedalMode,
                                                           int pedalCount,
                                                           int buttonCount,
                                                           int unusedPedal,
                                                           int unusedButton,
                                                           int momentary1Mode,
                                                           int momentary2Mode,
                                                           int momentary3Mode,
                                                           int ladderMode,
                                                           int analogMomentaryMode)
{
  if (pedal1 < 0 || button1 < 0 || pedal1 >= pedalCount || button1 >= buttonCount) {
    return {false, 0, 0, "Unmapped control"};
  }

  if (pedal2 != unusedPedal || button2 != unusedButton) {
    return {false, 0, 0, "Simultaneous control"};
  }

  if (!hardware_pedal_mode_is_virtual_pressable(pedalMode,
                                                momentary1Mode,
                                                momentary2Mode,
                                                momentary3Mode,
                                                ladderMode,
                                                analogMomentaryMode)) {
    return {false, 0, 0, "Unsupported pedal mode"};
  }

  return {true, static_cast<unsigned char>(pedal1), static_cast<unsigned char>(button1), ""};
}

inline const char* hardware_display_label(const char* tagOff,
                                          const char* tagOn,
                                          bool active,
                                          const char* fallback)
{
  const char* selected = active ? tagOn : tagOff;
  if (selected != 0 && selected[0] != 0) return selected;
  return fallback == 0 ? "" : fallback;
}

inline bool tap_tempo_timeout(unsigned long lastTap, unsigned long currentTime, unsigned long timeoutMs)
{
  return (currentTime - lastTap) > timeoutMs;
}

inline unsigned int tap_tempo_bpm_from_time(unsigned long lastTap, unsigned long currentTime)
{
  if (lastTap == 0 || currentTime <= lastTap) return 0;

  const unsigned long msInAMinute = 1000 * 60;
  return msInAMinute / (currentTime - lastTap);
}

template <typename Position, unsigned int N>
inline void tap_tempo_reset(unsigned long& lastTap, Position& currentReadingPos, unsigned int (&readings)[N])
{
  lastTap = 0;
  currentReadingPos = 0;
  for (unsigned int i = 0; i < N; i++) readings[i] = 0;
}

template <typename Position, unsigned int N>
inline unsigned int tap_tempo_average(Position currentReadingPos, const unsigned int (&readings)[N])
{
  unsigned int sum = 0;
  const unsigned int count = currentReadingPos < N ? currentReadingPos : N;
  if (count == 0) return 0;

  for (unsigned int i = 0; i < count; i++) sum += readings[i];
  return (sum + (count / 2 + 1)) / count;
}

template <typename Position, unsigned int N>
inline unsigned int tap_tempo_tap(unsigned long& lastTap,
                                  Position& currentReadingPos,
                                  unsigned int (&readings)[N],
                                  unsigned long currentTime,
                                  unsigned long timeoutMs)
{
  if (lastTap > 0) {
    if (tap_tempo_timeout(lastTap, currentTime, timeoutMs)) {
      tap_tempo_reset(lastTap, currentReadingPos, readings);
    }

    readings[currentReadingPos % N] = tap_tempo_bpm_from_time(lastTap, currentTime);
    currentReadingPos++;

    if (currentReadingPos >= 2) {
      lastTap = currentTime;
      return tap_tempo_average(currentReadingPos, readings);
    }
  }

  lastTap = currentTime;
  return 0;
}

} // namespace pedalino

#endif
