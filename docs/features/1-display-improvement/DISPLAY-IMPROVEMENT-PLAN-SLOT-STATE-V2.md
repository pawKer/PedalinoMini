## Plan: S3 Grid V2 (Slot State + Temporary Action Overlay)

### Summary
- Create a new plan doc alongside existing ones at `features/DISPLAY-IMPROVEMENT-PLAN-SLOT-STATE-V2.md`.
- Extend S3 grid behavior with explicit slot state control (default off) and a new display-only action `Set Slot State`.
- Reinstate the temporary action/value full-screen overlay for 1.5s on S3 (same interaction style as legacy behavior).
- Improve grid readability with larger 2-line cell text and fixed truncation.

### Public Interface Changes
- Add new action message type: `Set Slot State`.
- Action parameter contract:
1. `Code` = target slot (`1..6`)
2. `Value1` = slot state (`0=off`, `1=on`)
3. Display-only (no MIDI output)
- Add mapping support in action string/enums and web dropdown:
1. `ActionStringToEnum("Set Slot State")`
2. `ActionEnumToString(PED_ACTION_SET_SLOT_STATE)`
3. Actions UI `Send` includes `Set Slot State`
- UI labels for this action:
1. `Code` label -> `Slot`
2. `From/Value1` label -> `State (0/1)`

### Implementation Changes
1. **Runtime slot-state model**
- Add per-bank, per-slot runtime state (on/off + initialized flag).
- Default all slots to off on startup and when switching banks (runtime-only, not persisted).
- Render state rules:
1. Off/default -> white text on black
2. On -> inverted (black text on white)
- Keep `Actions[].Slot` as the only binding model for which actions feed each cell.

2. **Tag state semantics**
- For actions with two tags, interpret:
1. `tag1` as ON
2. `tag0` as OFF
- Preserve existing `###` substitution behavior for displayed labels.

3. **`Set Slot State` action execution**
- In action execution path, handle `Set Slot State` by updating runtime slot state only.
- Clamp slot from `Code` to `1..6`; clamp state from `Value1` to `0..1`.
- Exclude this action type from grid label source selection so it never occupies a display cell.
- Force its own slot binding to unassigned (`SLOTS`) on apply/save to avoid accidental rendering conflicts.

4. **S3 temporary overlay reinstatement (chosen UX)**
- Reintroduce full overlay for `~1500ms` using existing `endMillis2` timing mechanism.
- During overlay window:
1. Replace grid content with temporary action/value card
2. Show action name (`lastPedalName`) and relevant value substitution
3. Keep top bar visible
- After timeout, restore grid immediately.
- Integrate with current anti-flicker cache:
1. Bypass normal grid-cache short-circuit while overlay is active
2. Force one redraw when overlay ends so grid state resumes cleanly

5. **Grid text readability**
- Switch cell labels to 2-line centered larger font.
- Fixed truncation policy:
1. Max 2 lines
2. Fixed char cap per line
3. Ellipsis in second line on overflow
- Cache key includes both rendered lines + active state to prevent redraw flicker.

### Test Plan
1. **Default state**
- Fresh boot and bank switch: all cells render off (white on black) until state-changing action occurs.

2. **Tag ON/OFF**
- Actions with both tags flip visual state correctly (`tag1` -> on/inverted, `tag0` -> off/normal).

3. **`Set Slot State`**
- Configure action with `Code=1`, `Value1=0/1`; verify slot 1 toggles off/on reliably.
- Confirm no MIDI is sent by this action.
- Confirm it appears in UI/config and survives save/load/import/export.

4. **Overlay behavior**
- Trigger action updates and verify full temporary overlay appears for ~1.5s, then returns to grid.
- Verify value substitution (`###`) in overlay text.
- Verify no persistent blinking after overlay exit.

5. **Regression**
- Existing S3 top bar + grid behavior remains stable.
- Non-S3 display behavior unchanged.

### Assumptions
- Scope is S3 live display behavior only.
- Runtime slot state is intentionally non-persistent.
- Full-screen temporary overlay (not cell-only/top-bar-only) is the chosen UX.
- Existing slot schema is reused; no new config schema is introduced.
