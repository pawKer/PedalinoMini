### Plan: S3 Slot Border Colors by Control (V1)

#### Summary
Add a per-control color setting (configured in the **Controls** tab) and use it to render a **thicker colored border accent** on each S3 grid slot.  
Scope is **T-Display S3 only**. Existing behavior remains unchanged unless colors are explicitly configured.

#### Implementation Changes
1. **Control color model + persistence**
- Introduce a per-control color store as a separate runtime/persistent array (not by resizing `struct control`) to avoid profile-storage compatibility issues.
- Color format: `#RRGGBB` in JSON/UI; internal `uint32_t` RGB.
- Persist with profile save/load in both NVS and SPIFFS flows.
- Extend `Controls` JSON schema/config with optional `Color`.
- Backward compatibility:
  - Missing/legacy color values default to white.
  - Existing configs continue to load without migration errors.

2. **Web UI (Controls tab only)**
- Add one color picker per control card in the Controls page.
- Keep Apply/Save behavior aligned with existing controls editing.
- Update controls POST handler to parse/store control colors.
- Update config export/import so control colors round-trip.

3. **S3 display rendering**
- Apply to S3 2x3 grid path only.
- Use slot label-selected action’s control as the slot color source.
- Render a **thicker colored border accent** for each slot (instead of the thin line concept).
- Keep current active/inactive text inversion behavior unchanged.
- Fallback color is white if slot has no mapped action or control color is unset.
- For mixed-control slots, v1 fallback is deterministic: color from the same action used for slot label selection.

4. **Validation and compatibility**
- No hard validation blocking for mixed-control slots in v1.
- Preserve current behavior for non-S3 targets and non-grid views.
- Ensure schema and runtime stay aligned to prevent false frontend warnings.

#### Test Plan
1. **UI + persistence**
- Set distinct colors for several controls in Controls tab, Save, reboot, verify values persist.
- Export config and confirm `Controls[].Color` entries are present.
- Re-import exported config and verify colors are restored.

2. **Legacy compatibility**
- Load config with no `Controls[].Color`; verify no errors and white fallback is used.
- Confirm profile save after legacy load writes valid color defaults/values without breaking other fields.

3. **S3 rendering behavior**
- Verify each slot border uses the mapped control color.
- Verify slot ON/OFF visual state still works (text/background inversion unchanged).
- Verify mixed-control slot fallback follows label-selected action control color.
- Verify overlay behavior remains unchanged and no new flicker is introduced.

4. **Regression**
- Non-S3 display targets unaffected.
- Existing actions, slot-state action, bank switching, and web config pages continue to function.

#### Assumptions
- V1 scope is S3 live grid only.
- Color config lives only in Controls tab.
- Default unconfigured color is white.
- Mixed-control slot conflicts are tolerated in v1 with deterministic fallback, not blocked.
