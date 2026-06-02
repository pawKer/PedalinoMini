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
9. `TBD` - HX Stomp looper bank config refresh
10. `TBD` - Per-bank incoming MIDI actions
11. `TBD` - Config surface consistency checks
12. `TBD` - Unofficial custom build identity
13. `TBD` - Native unit test scaffold
14. `TBD` - Expanded native and Web/config contract tests

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

### `TBD` - Pending local documentation/workflow updates
- Added `ADDITIONAL-CONTEXT.md` at the repository root for persistent user-specific hardware layout notes.
- Updated `AGENTS.md` to reference `ADDITIONAL-CONTEXT.md` during startup/context gathering.
- Added slot-to-footswitch mapping and current HX looper bank placement notes to `ADDITIONAL-CONTEXT.md`.

### `TBD` - HX Stomp looper bank config refresh
- Replaced `Bank 2` (`:HX Looper`) in `current-cfg-230426.cfg` and `new-230426.cfg` with a focused HX Stomp looper layout on `Control 4..6`.
- Added dedicated looper sequences for `REC/ODUB`, `PLAY/STOP`, and `UNDO/REDO`, and cleared the extra looper sequence slots that were no longer used.
- Added `Control 3` `Long Press` as a dedicated HX looper `Clear Loop` action (`CC52`) so loop erase stays available without occupying one of the main three transport switches.
- Removed provisional HX looper incoming trigger rules for now, so Bank 2 currently runs as a local-state looper layout without receive-side sync.
- Compatibility note: `UNDO/REDO` remains effectively stateless in Pedalino, and the looper labels/slot states will not follow HX changes made from the HX itself or another controller until verified feedback mapping is added.

### `TBD` - Per-bank incoming MIDI actions
- Changed incoming MIDI trigger groups from one profile-wide ruleset to bank-scoped rules using `Global` bank `0` plus banks `1..20`.
- Updated runtime dispatch so incoming MIDI evaluates `Global` rules first, then the current bank, and can continue into the newly selected bank when a global trigger performs `Set Bank`.
- Added sparse per-bank incoming-trigger storage in RAM plus per-bank NVS persistence keys, while migrating older grouped `IncomingTriggers` entries without a trigger-level `Bank` into `Global`.
- Updated `/incoming-actions` to select and edit one bank at a time and added a `Duplicate Bank To` dropdown that replaces the destination bank's incoming rules.
- Hardened incoming trigger edits/config loads so MIDI dispatch skips while trigger storage is being resized or rewritten, without blocking profile load during boot.
- Changed legacy NVS `InTriggers` migration to read into allocated trigger storage instead of a large boot-time stack buffer.
- Bank reordering in `/actions` now carries each bank's incoming MIDI rules with the bank's actions and name.
- Updated JSON/schema support so each `IncomingTriggers` entry persists its owning bank.
- Compatibility note: legacy flat `IncomingActions` still requires manual recreation, but prior grouped `IncomingTriggers` configs now load into `Global` automatically when their trigger-level `Bank` field is missing; this migration was confirmed on the LilyGO T-Display S3 with existing stored rules.

### `TBD` - Config surface consistency checks
- Added a lightweight Python validation script for action string mappings, JSON schema action enums, and key Web UI labels.
- Added unit coverage for the validation helpers and wired the same local validation script into CI before PlatformIO firmware/filesystem builds.
- Compatibility note: no firmware behavior, config format, or web UI behavior changed.
- Validation: `python -m unittest scripts.test_validate_config_surfaces`, `python scripts/validate_config_surfaces.py`, and `pio run -e lilygo-t-display-s3`.

### `TBD` - Unofficial custom build identity
- Added README and installer disclaimers for `PedalinoMini 6 T-Display S3 Custom (Unofficial)` builds.
- Updated generated and packaged ESP Web Tools manifest names to show the unofficial custom build label.
- Added a local unit test that checks README, installer, manifest identity, and guards against changing runtime connectivity names.
- Compatibility note: no firmware runtime behavior, hostname, BLE MIDI name, USB MIDI identity, or config format changed.
- Validation: `python -m unittest scripts.test_validate_config_surfaces scripts.test_custom_build_identity`, `python scripts/validate_config_surfaces.py`, and `pio run -e lilygo-t-display-s3`.

### `TBD` - Native unit test scaffold
- Added a PlatformIO `native` environment backed by a project-local `native_host` board definition for host-side Unity tests.
- Extracted dependency-free core helpers for `map2` and incoming MIDI trigger matching into `src/PedalinoCoreLogic.h`, and wired the existing firmware paths through those helpers.
- Added native Unity coverage for `map2` endpoint/rounding/zero-width behavior and incoming MIDI trigger matching for CC/PC, Any/Exact values, Any channel, wrong channel/number, and unsupported message types.
- Wired native unit tests into build and CodeQL CI, and added local validation plus native unit tests to the release workflow before firmware packaging.
- Compatibility note: intended firmware behavior and config format are unchanged; this is a testability and CI-gating change.
- Validation: `python -m unittest scripts.test_validate_config_surfaces scripts.test_custom_build_identity`, `python scripts/validate_config_surfaces.py`, `$env:TMPDIR = "C:\tmp"; pio test -e native`, and `$env:TMPDIR = "C:\tmp"; pio run -e lilygo-t-display-s3`.

### `TBD` - Expanded native and Web/config contract tests
- Expanded host-side native Unity coverage for analog response mapping, display overlay label selection, slot-state tag decisions, incoming MIDI dispatch order and bank scope handling, RGB LED order swaps, chunked Web UI page trimming, and tap-tempo averaging/reset behavior.
- Added Python contract coverage for `/incoming-actions` route/form fields, duplicate-bank action wiring, sequence-name UI/config contracts, legacy incoming-action import warnings, `SequenceNames` schema/runtime limits, and current plus legacy `IncomingTriggers` fixture support.
- Wired the new Web/config contract tests into build, CodeQL, and release workflows alongside the existing local validation tests.
- Compatibility note: intended firmware behavior and config format are unchanged; production paths now delegate more small decisions to dependency-free helpers so they can be regression-tested on the native host target.
- Validation: `python -m unittest scripts.test_validate_config_surfaces scripts.test_custom_build_identity scripts.test_web_config_contracts`, `python scripts/validate_config_surfaces.py`, `$env:TMPDIR = "C:\tmp"; pio test -e native`, and `$env:TMPDIR = "C:\tmp"; pio run -e lilygo-t-display-s3`.


## Validation
- Latest verified build: `$env:TMPDIR = "C:\tmp"; pio run -e lilygo-t-display-s3` (success, 2026-06-02, expanded native and Web/config contract tests worktree).
- Latest native test run: `$env:TMPDIR = "C:\tmp"; pio test -e native` (21 Unity tests passed, 2026-06-02).
- Latest local Python validation: `python -m unittest scripts.test_validate_config_surfaces scripts.test_custom_build_identity scripts.test_web_config_contracts` (18 tests passed, 2026-06-02).
- Latest device check: existing grouped incoming MIDI rules migrated into `Global` bank `0` and no longer caused a reboot loop on LilyGO T-Display S3.
