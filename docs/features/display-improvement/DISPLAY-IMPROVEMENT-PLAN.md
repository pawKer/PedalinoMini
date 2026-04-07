# Final Plan: T-Display S3 Fixed 2x3 Grid Using Existing Action Slots

## Summary
Implement a new T-Display S3 live layout with:
1. Top bar keeps WiFi/profile/battery status and also shows current bank name.
2. Main display is always a fixed 2x3 grid of tag blocks.
3. Grid content is driven by existing `Actions[].Slot` (reuse current model; no new `DisplaySlots` schema).
4. Slot highlight behavior is event-driven: last-triggered action for a slot owns the visible label state.

## Key Implementation Changes
1. **Display layout behavior**
- Replace current mixed “pedal names + center bank/value alternation” with a permanent 6-cell grid.
- Render bank name in top bar center; keep status icons.
- For top-bar bank text, strip only leading legacy prefix markers (`:` / `.`) for display, but do not modify stored bank names.
- If bank name exceeds available space, truncate with ellipsis (no marquee, no wrapping).

2. **Slot-driven label model (reuse existing fields)**
- Treat `Actions[].Slot` as display binding: `0..5` map to grid cells, `6` means unassigned.
- No new config section/type is introduced; public behavior change is that existing slot assignments now directly control on-device grid labels.
- For each cell:
  - Idle baseline label comes from the first sorted action mapped to that slot.
  - Runtime label/state changes follow action activity, with **last-triggered action wins** when multiple actions share a slot.
  - Preserve existing `NameOff/NameOn` and `###` substitution behavior.

3. **One-time migration for existing configs**
- On profile load, detect uninitialized slot layouts per bank (banks 1..20 only): all actions assigned to slot `0` and no use of slots `1..5`.
- Auto-distribute first 6 sorted actions to slots `0..5`; set remaining actions to slot `6`.
- Skip migration for bank 0 (global bank).
- Persist migrated result once so migration does not repeat on subsequent boots.

4. **Web UI and persistence alignment**
- Keep using current Actions-page slot controls as the single source of truth.
- No new user-facing storage structure; only runtime/render semantics change.
- Ensure apply/save/import/export continue to preserve slot assignments exactly.

## Test Plan
1. Load your provided `current-conf-070426.cfg` and verify migration outcome:
- Bank 1 (`slot0:12`) becomes distributed across `0..5` with remainder unassigned.
- Bank 2 (`slot0:3`) distributes first three slots and leaves others empty.
- Bank 0 remains unchanged (`slot6` actions stay unassigned in grid).

2. Validate top bar rendering:
- WiFi/profile/battery still visible.
- Bank names like `:HX Stomp` render as `HX Stomp`.
- Long names truncate with ellipsis without icon overlap.

3. Validate runtime slot behavior:
- Multiple actions sharing one slot: most recently triggered action label/state appears.
- `NameOff/NameOn` switching works.
- `###` dynamic value replacement still updates correctly.

4. Persistence and compatibility:
- Reboot after migration and confirm slots are stable (no repeated reshuffle).
- Save/apply in Actions UI and confirm slot edits reflect immediately on grid.
- Import/export round-trip preserves slot values and grid behavior.

## Assumptions
- Scope is T-Display S3 live view only for v1.
- Main area is always fixed 2x3 grid (no timed alternate mode).
- Existing slot field is intentionally reused; no parallel display-binding schema is added.
- Global bank is excluded from auto-distribution migration.
