# Fork Release Notes

## Branch `custom-open-source` (fork-specific notes)

### Commit Inventory (fork-specific commits)
1. `f094f50` (2026-04-07) - Added display slots feature, fixed displayed tag on action, added new slot state action
2. `d3af9fd` (2026-04-10) - Slot border colours + display customization tab on web UI
3. `bb1c5cc` (2026-04-10) - Added sequence names + docs reorganization
4. `7cef3f1` (2026-04-11) - Actions on incoming MIDI messages (limited set)
5. `a5cb5ca` (2026-04-23) - Map MIDI in pin to a separate unused pedal pin
6. `e9c40af` (2026-04-23) - Fixing MIDI in actions + display, fixing tags display correctly
7. `6dcd6cf` (2026-04-23) - Update feature doc
8. `TBD` - Pending local documentation/workflow updates

### `f094f50` - Display state model + `Set Slot State` action
- Added new action type `PED_ACTION_SET_SLOT_STATE` and string mapping in config serialization/deserialization.
- Added runtime slot display state arrays (`slotDisplayState`, `slotDisplayInitialized`) and controller helpers to set/reset state.
- Added runtime handling for `Set Slot State` action in controller execution.
- Updated Actions Web UI to expose and sanitize `Set Slot State`.
- Updated the S3 display rendering path for slot-state-aware overlay and grid behavior.

### `d3af9fd` - Display customization tab + slot border colors
- Added profile-scoped slot border colors via `slotBorderColor[SLOTS]`.
- Added persistence through JSON, schema, and NVS.
- Added `/display` Web UI with 6 slot color pickers and preview.

### `bb1c5cc` - Sequence names + docs reorganization
- Added profile-scoped sequence names with JSON/schema/NVS support.
- Updated Actions and Sequences UI to show `N (Name)` labels.
- Reorganized feature docs under numbered folders in `docs/features/`.

### `7cef3f1` - Incoming MIDI Actions V2
- Added grouped incoming trigger support through `IncomingTriggers`.
- Added `/incoming-actions` Web UI and runtime dispatcher support.
- Added config/schema/NVS round-trip support for incoming trigger groups.

### `a5cb5ca` - LilyGO T-Display S3 DIN MIDI pin split
- Split LilyGO T-Display S3 DIN MIDI input and output onto separate GPIOs.
- `DIN_MIDI_OUT_PIN = GPIO 1`
- `DIN_MIDI_IN_PIN = GPIO 2`

### `e9c40af` - Incoming MIDI and display follow-up fixes
- Removed implicit receive-side LED matching from incoming MIDI handlers.
- Incoming LED/display changes on receive now depend on explicit `IncomingTriggers`.
- Fixed `Show Incoming` so it correctly controls device overlays.
- Improved overlay label selection to prefer configured action tags.
- Increased incoming trigger action capacity to 8.

### `6dcd6cf` - Feature doc update
- Updated the incoming MIDI Actions feature doc to reflect the latest behavior.


## Validation
- Latest verified build: `pio run -e lilygo-t-display-s3` (success).
