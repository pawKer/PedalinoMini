#ifndef _PEDALINO_CORE_LOGIC_H
#define _PEDALINO_CORE_LOGIC_H

namespace pedalino {

inline long map2(long x, long in_min, long in_max, long out_min, long out_max)
{
  const long dividend = out_max - out_min;
  const long divisor = in_max - in_min;
  const long delta = x - in_min;

  if (x == in_min) return out_min;
  if (x == in_max) return out_max;

  return (divisor == 0 ? (x <= in_min ? out_min : out_max) : (delta * dividend + (divisor / 2)) / divisor + out_min);
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

} // namespace pedalino

#endif
