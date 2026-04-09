### Plan: Move S3 Border Colors From Controls to UI Slots + New Display Tab

#### Summary
Replace control-linked S3 slot border colors with profile-wide slot-linked colors (`6` slots per profile), add a dedicated **Display** tab in Web UI to edit them, and include a **basic live mock-up** (2x3 preview) in that tab.  
Control colors are removed entirely from this feature path.

#### Key Changes
- **Runtime + persistence model**
  - Replace `controlColor[CONTROLS]` with `slotBorderColor[SLOTS]` (6 profile-wide colors).
  - Default all slot colors to white (`#FFFFFF`) on init and when missing.
  - Persist slot colors in both NVS and SPIFFS profile flows.
  - Store slot colors in profile JSON as a new top-level `DisplaySlots` array (6 items, slot+color).

- **Schema + config interface updates**
  - Add `DisplaySlots` schema section with fixed 6 slots and color strings.
  - Remove `Controls[].Color` from schema and JSON export generation.
  - Keep import tolerant: old configs containing `Controls[].Color` remain loadable (field ignored by new logic).
  - NVS: stop reading/writing `CtrlColor`; use new slot-color key.

- **S3 rendering behavior**
  - In S3 grid draw path, border color source becomes `slotBorderColor[slotIndex]` only.
  - Remove any action/control-based border color derivation.
  - Keep existing slot text/state rendering logic and current accent thickness behavior unchanged.

- **Web UI changes (new tab + mock-up)**
  - Add new nav tab/page: `Display` (`/display`, GET+POST).
  - Page content:
    - 6 slot color pickers (Slot 1..6).
    - Basic 2x3 live mock-up card reflecting selected border colors immediately (client-side JS).
  - Mock-up is intentionally simple: static slot labels (e.g., Slot 1..6), no full runtime/action simulation.
  - Apply/Save behavior matches existing pages (Apply = runtime only, Save = persist profile).

- **Controls tab cleanup**
  - Remove control color picker UI from Controls page.
  - Remove controls POST parsing for `controlcolor-*`.
  - Controls page continues handling pedal/button/LED mapping only.

#### Public Interfaces / Types
- New HTTP endpoints: `GET /display`, `POST /display`.
- New profile config field: `DisplaySlots` (6-slot color mapping).
- Removed feature field: `Controls[].Color` (no longer part of supported editing/export path).

#### Test Plan
1. **Display tab UX**
   - Open `/display`, verify 6 pickers render and mock-up updates instantly per picker change.
2. **Apply/Save persistence**
   - Apply colors and confirm S3 borders update immediately.
   - Save, reboot, confirm colors persist for the same profile.
3. **Profile behavior**
   - Change profile A/B/C and verify each profile has its own 6-slot color set.
4. **Config round-trip**
   - Export config includes `DisplaySlots`.
   - Import exported config restores same slot colors.
5. **Legacy compatibility**
   - Import older config containing `Controls[].Color`; load succeeds and defaults to white unless `DisplaySlots` exists.
6. **Rendering correctness**
   - Mixed-control slots still show one border color per slot from slot mapping only.
   - Non-S3 targets remain unchanged.

#### Assumptions (Locked)
- Border color source is fully replaced by slot colors (no control fallback).
- Scope is profile-wide slot colors (not per-bank).
- Migration default is all-white slots.
- New tab is dedicated top-level **Display** tab.
- Mock-up is basic/static in v1 (not data-driven from live actions).
