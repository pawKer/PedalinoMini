# Revised Plan: T-Display S3 2x3 Grid (Slim V1)

## Summary
Deliver the desired display redesign with minimal risk:
1. Keep top bar status icons (WiFi/profile/battery) and show current bank name in top bar.
2. Replace current main area with a fixed 2x3 block grid.
3. Reuse existing `Actions[].Slot` as the only source for block assignment.

## Key Implementation Changes
1. **Layout**
- Update T-Display S3 live screen to a permanent 2x3 grid.
- Keep existing top bar icon rendering and add bank name in available top-bar text area.
- Remove timed center-area switching (bank/value alternation) on this screen.

2. **Slot mapping behavior**
- Use existing action slot values: `0..5` map to the 6 blocks, `6` means hidden/unassigned.
- For v1, each block shows the first sorted action assigned to that slot (deterministic and simple).
- Preserve current tag rendering behavior (`NameOff`/`NameOn` usage and `###` replacement where already supported).

3. **Web UI and persistence**
- Keep current Actions page slot editor as-is (no new UI model).
- No new config schema; no new storage objects.
- Existing apply/save/import/export flows continue to persist slot values exactly as today.

## Follow-Up Future Actions
1. Add one-time auto-migration for legacy configs where all actions are concentrated on slot `0`, with safe persistence rules and bank scoping.
2. Add runtime arbitration for slots with multiple mapped actions using “last-triggered action wins”.
3. Add bank-name display polish in top bar (prefix cleanup for `:`/`.` and ellipsis logic for long names).

## Test Plan
1. Verify T-Display S3 shows top bar icons plus bank name and fixed 2x3 grid.
2. Verify blocks reflect existing `Actions[].Slot` assignments from current config.
3. Verify slot edits in Actions UI reflect on device after apply/save/reboot.
4. Verify no regressions in profile load, bank switch, and normal action triggering.
5. Smoke-check one non-S3 target to confirm unchanged behavior.

## Assumptions
- Scope is T-Display S3 live view only for v1.
- No config migration in v1.
- No new storage schema in v1.
- When multiple actions share a slot, v1 uses first sorted action only.
