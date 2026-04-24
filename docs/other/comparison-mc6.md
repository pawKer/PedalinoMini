# Create `docs/MC6-PRO-COMPARISON.md`

## Summary
Create a new top-level reference doc at `docs/MC6-PRO-COMPARISON.md` that captures:
- An official-docs-based software feature comparison between the Morningstar MC6 Pro and PedalinoMini.
- A capability matrix showing `Present`, `Partial`, or `Missing` status in PedalinoMini.
- A prioritized parity roadmap focused on making your 6-switch LilyGO T-Display S3 behave more like an MC6 Pro.
- Concrete usage examples for each prioritized feature so the document is useful as a future planning reference, not just a checklist.

## Document Content
1. **Purpose and scope**
- State that the comparison is based on Morningstar official manuals and the current PedalinoMini repo state.
- State that the comparison is software-feature focused, not hardware-feature focused.
- Note that the goal is “MC6 Pro-like workflow” rather than exact hardware replication.

2. **High-level comparison table**
- Include one compact table with rows like:
  - Preset architecture
  - Action types
  - Toggle/shift behavior
  - Visual preset states
  - MIDI message types
  - MIDI routing / remote control
  - Expression handling
  - Computer control / keystrokes
  - Editor / configuration workflow
  - Device-specific integrations
  - Safety / utility functions
- Use status values:
  - `Present`
  - `Partial`
  - `Missing`
- Keep the PedalinoMini column grounded in current repo capabilities only.

3. **Current strengths of PedalinoMini**
- Add a short section highlighting where PedalinoMini is already strong:
  - DIY input flexibility
  - Network MIDI / OSC support
  - On-device web UI and config import/export
  - S3 display slot system
  - ESP32-S3 keyboard support

4. **Priority roadmap**
- Add a section named `Recommended Priorities`.
- Order the items exactly like this:
  1. Preset / toggle-state model
  2. Page semantics and faster navigation
  3. Device/control utility message types
  4. General incoming MIDI message conversion
  5. Clearer per-state display and LED semantics
- For each item include:
  - Why it matters for MC6-style workflow
  - What PedalinoMini already has that can be reused
  - What is still missing conceptually

5. **Usage examples**
- Add one short real-world example under each priority:

### 1. Preset / toggle-state model
Example:
- Switch A controls `HX Stomp Delay`.
- First press sends `CC#20 value 127`, lights the LED, and changes the slot label to `Delay On`.
- Second press sends `CC#20 value 0`, restores the LED off state, and changes the slot label to `Delay Off`.
- The important MC6-like behavior is that the controller remembers whether the switch is currently in Position 1 or Position 2.

### 2. Page semantics and faster navigation
Example:
- Page 1 contains your main stomp effects.
- Page 2 contains amp / snapshot / utility functions.
- Holding or double-tapping a switch flips to Page 2 without changing bank.
- This lets one physical 6-switch layout behave like 12 or more logical switches, similar to the MC6 Pro bank/page workflow.

### 3. Device/control utility message types
Example:
- One switch sends:
  - a delayed PC to load an HX Stomp preset,
  - then a delayed CC to sync bypass state,
  - then a local display update.
- Another switch acts like `Engage Preset B` by triggering another stored logical action block instead of duplicating all messages.
- This reduces config duplication and makes complex rigs easier to maintain.

### 4. General incoming MIDI message conversion
Example:
- HX Stomp sends incoming `CC#51 value 127`.
- PedalinoMini converts that into:
  - local LED color update,
  - slot state update,
  - optional outgoing CC/PC to another device,
  - optional page/preset/toggle update.
- This is more MC6-like than the current narrower incoming-trigger model because incoming MIDI becomes a reusable automation/control layer.

### 5. Clearer per-state display and LED semantics
Example:
- Each logical switch has:
  - off label,
  - on label,
  - optional shifted label,
  - off LED color,
  - on LED color,
  - optional shifted color.
- On the T-Display S3, the active state is obvious without needing to infer it from unrelated action tags.
- This gets closer to the MC6 Pro experience where visual state is part of the preset model, not just a side effect of raw messages.

6. **Recommended interpretation**
- Add a short closing section saying:
  - The biggest parity gap is not raw MIDI support.
  - The biggest parity gap is the controller model: presets, toggle states, pages, and stateful UI behavior.
  - PedalinoMini already has enough building blocks that this can likely be added incrementally rather than through a full rewrite.

## Sources to cite in the doc
- Morningstar official:
  - `MC6 PRO` manual
  - `MC6 PRO User Manual`
  - `Message Type List`
  - `Action Type List`
- PedalinoMini local sources:
  - `README.md`
  - `docs/AI_CONTEXT.md`
  - `docs/RELEASE-NOTES.md`
  - `docs/features/5-midi-in-actions/midi-in-actions.md`

## Assumptions
- The file should be a durable reference doc, not a feature plan for immediate implementation.
- It should stay concise and comparison-oriented, with examples focused on your HX Stomp / 6-footswitch use case.
- It should avoid claiming exact MC6 Pro parity where the repo only supports a similar but not identical behavior.
