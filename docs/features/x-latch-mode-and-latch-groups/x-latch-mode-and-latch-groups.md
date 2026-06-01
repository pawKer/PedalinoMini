# Control-Level Latch Mode + Exclusive Latch Groups

## Summary
- The repo already has physical-pedal latch behavior (`LatchEmulation`, latch pedal modes) plus runtime LED/slot state, but it does not have a logical control-state owner. This feature should be built at the Control layer, not by extending pedal-level latch emulation.
- Add a new logical control behavior model so a control can be `Momentary` or `Latch`, and optionally belong to an exclusive latch group where only one member is active at a time.
- Scope v1 to single-input controls only (`Pedal2/Button2` unset). Existing physical latch pedal modes stay unchanged.
- Size: medium-large and cross-cutting. Expect roughly `4-6 engineer-days` including runtime work, web/config/schema updates, incoming sync hooks, docs, and one `lilygo-t-display-s3` validation build.

## Implementation Changes
- Add new control-level config arrays rather than changing `struct control`, to avoid breaking the raw `Controls` NVS blob:
  - `controlBehavior[CONTROLS]`: `Momentary` or `Latch`
  - `controlLatchGroup[CONTROLS]`: `0` = none, `1..16` = exclusive group
  - `latchedControlState[BANKS][CONTROLS]`: runtime on/off state, RAM-only
- Extend `Controls` JSON/schema/UI with:
  - `Behavior`: `"Momentary" | "Latch"`
  - `LatchGroup`: `0..16`
  - Defaults: `Momentary`, group `0`
- In the button event path, resolve the control first, then apply logical behavior:
  - `Momentary`: keep the current flow
  - standalone `Latch`: physical press toggles state and dispatches logical `Press` when turning on, logical `Release` when turning off
  - grouped `Latch`: physical press always selects this control, clears peers in the same group and bank, and dispatches logical `Press` for the selected control
  - pressing the already-active grouped control keeps it active and re-runs its on/select path
- Peer deselection in groups is visual/state-only in v1:
  - clear peer latch state
  - update peer LED and slot visuals to off
  - do not send explicit off MIDI for displaced peers
- Add a shared visual helper such as `apply_control_visual_state(bank, control, active)` and use existing action metadata (`color0/color1`, `tag0/tag1`, `slot`) as the source of truth. No separate per-control label/color model in v1.
- Replace bank-entry reset-only behavior with a rebuild step:
  - clear transient slot state
  - reapply latched control visuals for the active bank
  - preserve latch/group selection across bank changes in the same session
- Add incoming sync hooks with a new incoming target action `Set Control State`:
  - UI/JSON contract: `Action="Set Control State"`, `Control=1..100`, `State=0|1`
  - incoming sync updates local latch/group state and visuals only; it must not echo outgoing MIDI
  - `State=1` selects/activates the control; grouped controls also clear peers
  - `State=0` turns off standalone latch controls; for grouped controls it is ignored in v1
  - keep the current incoming-trigger NVS size stable by storing the target control in an existing per-action numeric field and only changing the JSON/UI label/meaning for this action type
- Validation rules:
  - if a control has a second input configured, coerce `Behavior` to `Momentary` and `LatchGroup` to `0`
  - if multiple latch/group controls share the same single physical input, keep the lowest-numbered control stateful and coerce the rest to `Momentary` + group `0`
- Update docs and release notes in the same work:
  - feature doc under `docs/features/`
  - `FORK-RELEASE-NOTES.md`

## Effort
- Runtime control-state model and event routing: `1.5-2 days`
- Visual rebuild and bank/profile integration: `0.5-1 day`
- Config/schema/web UI/persistence: `1-1.5 days`
- Incoming sync hook: `0.5-1 day`
- Validation, config smoke test, docs, release notes: `0.5 day`

## Test Plan
- Standalone latch:
  - one control in `Latch` mode with a `Press & Release` action toggles on/off MIDI, LED, and slot state across repeated presses
- Press-only standalone latch:
  - repeated presses still flip local LED/slot state even if the config has no explicit off MIDI path
- Exclusive group:
  - three grouped controls for HX snapshots; only one LED/slot is active at a time
  - selecting a new member clears peers visually without sending peer off MIDI
  - pressing the active member again keeps it selected and re-sends its select message
- Bank behavior:
  - switching away and back restores latch/group visual state in the same session
  - reboot/profile reload resets latch/group state to defaults until a press or incoming sync sets it
- Incoming sync:
  - `Set Control State` can turn a standalone latch on/off
  - `Set Control State` can select a grouped member and clear peers without MIDI echo
- Regression:
  - momentary controls, physical latch pedal modes, `LatchEmulation`, `Set Slot State`, and bank switching still behave as before
  - `pio run -e lilygo-t-display-s3`

## Assumptions And Defaults
- `Latch` is a logical Control feature, not a replacement for physical latch pedals.
- V1 targets single-input controls only.
- The local toggle trigger is physical `Press`.
- Group behavior is radio-style: at most one active member, no all-off state from local presses.
- Peer deselection is visual/state-only, chosen to fit HX snapshot-style UX.
