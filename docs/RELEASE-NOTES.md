# Release Notes

## Branch `custom-open-source` (vs `origin/master`)

### Commit Inventory (all commits on this branch)
1. `f094f50` (2026-04-07) - Added display slots feature, fixed displayed tag on action, added new slot state action
2. `d3af9fd` (2026-04-10) - Slot border colours + display customization tab on web UI

### `f094f50` - Display state model + `Set Slot State` action
- Added new action type `PED_ACTION_SET_SLOT_STATE` and string mapping in config serialization/deserialization.
- Added runtime slot display state arrays (`slotDisplayState`, `slotDisplayInitialized`) and controller helpers to set/reset state.
- Added runtime handling for `Set Slot State` action in controller execution (display-only behavior).
- Updated action sorting to preserve/carry `slot` metadata during swaps.
- Updated bank-switch paths to reset slot display state when bank changes.
- Updated Actions Web UI:
  - Added `Set Slot State` in Send dropdown.
  - Added action-specific label/enable-disable behavior (`Slot`, `State (0/1)`).
  - Sanitized `Set Slot State` on apply/save (forces non-rendering slot usage and clamps parameters).
- Updated S3 display rendering path:
  - Added top-bar bank label rendering.
  - Reworked overlay/grid flow and slot label arbitration (including tag-based state semantics).
- Added supporting docs and session notes in `docs/`.

### `d3af9fd` - Display customization tab + slot border colors
- Introduced profile-scoped slot border color model:
  - New runtime global `slotBorderColor[SLOTS]`.
  - Default fallback to white.
- Added persistence for slot colors:
  - SPIFFS/JSON export/import via top-level `DisplaySlots`.
  - NVS profile key `SlotColor`.
- Added schema support:
  - Added `DisplaySlots` in `data/schema.json`.
  - Included `Set Slot State` in action message enum list in schema.
- Added new Web UI Display page:
  - New endpoints `GET /display`, `POST /display`.
  - 6 slot color pickers + basic 2x3 preview.
  - Apply/Save behavior aligned with existing profile workflow.
- Updated S3 grid rendering:
  - Border accent now sourced from `slotBorderColor[slot]`.
  - Added thicker accent behavior for custom (non-white) colors.
  - Improved cell text fitting/wrapping logic.

## Unreleased (working tree, not yet committed)
- Sequence Names feature has been implemented locally but is currently uncommitted:
  - New per-profile sequence names (`max 16`), shown as `N (Name)` in Actions/Sequences selectors.
  - New config/schema/NVS support: `SequenceNames` + `SeqNames`.
  - Sequences page includes editable `sequencename` input.
- Incoming MIDI Actions V2 (grouped triggers) has been implemented locally but is currently uncommitted:
  - Data model upgraded from flat rules to grouped triggers:
    - `INCOMING_TRIGGERS_MAX=64`
    - `INCOMING_TRIGGER_ACTIONS_MAX=4`
    - `incomingTriggers[]` + `incomingTriggerCount`
  - One incoming CC/PC trigger can now execute multiple actions in stored order.
  - Dispatcher keeps existing precedence and now executes action chains per matched trigger group.
  - `/incoming-actions` UI now supports Trigger Groups with per-group Add Action/Delete Group and per-action delete.
  - Config/schema/NVS migrated to grouped shape:
    - JSON: `IncomingTriggers`
    - NVS: `InTriggers`, `InTrigCnt`
  - Legacy flat `IncomingActions` is treated as manual migration only (warning shown, no auto-convert).

## Validation
- Latest verified build: `pio run -e lilygo-t-display-s3` (success).
