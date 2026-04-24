# HX Stomp Looper Bank Configuration

## Summary
Create a new dedicated 6-switch bank for the HX Stomp one-switch looper workflow, separate from the current partial `Bank 2` setup.  
This bank is designed for:
- your 6 main footswitch controls (`Control 1..6`)
- HX Stomp on MIDI channel `1`
- a full looper layout
- future incoming-MIDI sync for LEDs/display, even if some buttons temporarily rely on local step-by-step state

Use bank name:
- `:HX Looper`

Use display slots:
- `Control 1 -> Slot 1`
- `Control 2 -> Slot 2`
- `Control 3 -> Slot 3`
- `Control 4 -> Slot 4`
- `Control 5 -> Slot 5`
- `Control 6 -> Slot 6`

## Bank Layout
Use this left-to-right switch mapping:

1. **Control 1: Record / Overdub**
- Action type: `Step by Step+`
- Sequence: dedicated looper sequence
- Display tags: `REC` / `ODUB`
- MIDI behavior:
  - Step 1: `CC60 value 127` = Record
  - Step 2: `CC60 value 0` = Overdub
- Purpose: first press starts recording, next press goes to overdub, then alternates locally

2. **Control 2: Play / Stop**
- Action type: `Step by Step+`
- Sequence: dedicated looper sequence
- Display tags: `PLAY` / `STOP`
- MIDI behavior:
  - Step 1: `CC61 value 127` = Play
  - Step 2: `CC61 value 0` = Stop
- Purpose: one-button transport control matching HX looper workflow

3. **Control 3: Undo / Redo**
- Action type: `Step by Step+`
- Sequence: dedicated looper sequence
- Display tags: `UNDO` / `REDO`
- MIDI behavior:
  - Step 1: `CC63 value 127`
  - Step 2: `CC63 value 127`
- Purpose: same HX command both times, with Pedalino toggling the label locally for usability

4. **Control 4: Play Once**
- Action type: direct `Control Change`
- Display label: `ONCE`
- MIDI behavior:
  - `CC62 value 127`
- Purpose: stateless one-shot playback command

5. **Control 5: Forward / Reverse**
- Action type: `Step by Step+`
- Sequence: dedicated looper sequence
- Display tags: `FWD` / `REV`
- MIDI behavior:
  - Step 1: `CC65 value 127` = Reverse
  - Step 2: `CC65 value 0` = Forward
- Purpose: default display state is forward, first press flips to reverse

6. **Control 6: Full / Half Speed**
- Action type: `Step by Step+`
- Sequence: dedicated looper sequence
- Display tags: `FULL` / `HALF`
- MIDI behavior:
  - Step 1: `CC66 value 127` = Half speed
  - Step 2: `CC66 value 0` = Full speed
- Purpose: default display state is full speed, first press flips to half speed

## Supporting Sequences
Create five dedicated sequences for the stateful looper buttons:

- `Sequence A`: Record / Overdub
  - Step 1: `Control Change`, ch `1`, code `60`, value `127`
  - Step 2: `Control Change`, ch `1`, code `60`, value `0`

- `Sequence B`: Play / Stop
  - Step 1: `Control Change`, ch `1`, code `61`, value `127`
  - Step 2: `Control Change`, ch `1`, code `61`, value `0`

- `Sequence C`: Undo / Redo label helper
  - Step 1: `Control Change`, ch `1`, code `63`, value `127`
  - Step 2: `Control Change`, ch `1`, code `63`, value `127`

- `Sequence D`: Forward / Reverse
  - Step 1: `Control Change`, ch `1`, code `65`, value `127`
  - Step 2: `Control Change`, ch `1`, code `65`, value `0`

- `Sequence E`: Full / Half Speed
  - Step 1: `Control Change`, ch `1`, code `66`, value `127`
  - Step 2: `Control Change`, ch `1`, code `66`, value `0`

Initialize each `Step by Step+` action so the **first press sends the left-to-right “engage” command shown above**:
- `REC`
- `PLAY`
- `UNDO`
- `REV`
- `HALF`

## Visual / UX Defaults
- Keep the bank fully dedicated to looper control; do not mix in bank navigation actions on these 6 switches.
- Use short labels that fit well on the T-Display S3:
  - `REC`, `ODUB`, `PLAY`, `STOP`, `UNDO`, `REDO`, `ONCE`, `FWD`, `REV`, `FULL`, `HALF`
- Recommended LED intent:
  - Record/Overdub: red
  - Play/Stop: green
  - Undo/Redo: amber
  - Play Once: blue
  - Forward/Reverse: purple
  - Full/Half: cyan
- Treat `PLAY`, `FWD`, and `FULL` as the default baseline display states when no sync has yet been applied.

## Future Sync Compatibility
Design this bank so it can later be upgraded with incoming MIDI feedback without changing the switch roles:
- `Record / Overdub`, `Play / Stop`, `Forward / Reverse`, and `Full / Half` should later map to slot-state sync if HX return MIDI is added.
- `Undo / Redo` should remain best-effort because HX exposes this as a shared command rather than two separate outbound commands.
- `Play Once` should stay stateless.

When incoming sync is added later, replace local-only display assumptions before removing the switch layout itself.

## Test Plan
1. Enter the looper bank and verify all 6 labels appear in the intended slot order.
2. Press `REC` twice and confirm HX receives `CC60 127`, then `CC60 0`.
3. Press `PLAY` twice and confirm HX receives `CC61 127`, then `CC61 0`.
4. Press `UNDO`, then press again and confirm the same `CC63 127` is sent both times while the local label alternates.
5. Press `ONCE` and confirm `CC62 127` is sent every time with no toggle state.
6. Press `REV`, then again, and confirm `CC65 127`, then `CC65 0`.
7. Press `HALF`, then again, and confirm `CC66 127`, then `CC66 0`.
8. Power cycle or bank-switch away and back; confirm the bank still loads cleanly and the labels remain readable on the S3 display.
9. Note as an accepted v1 limitation: local toggle labels may drift if HX looper state is changed from the HX itself or another controller.

## Assumptions
- HX Stomp looper commands use:
  - `CC60` Record/Overdub
  - `CC61` Play/Stop
  - `CC62` Play Once
  - `CC63` Undo/Redo
  - `CC65` Forward/Reverse
  - `CC66` Full/Half Speed
- HX Stomp listens on MIDI channel `1`.
- The target should be a fresh looper-bank design, not a cleanup of the current partial `Bank 2`.
- A dedicated looper bank is preferred over mixing looper controls with bank/profile navigation.
