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
12. `TBD` - Native unit test scaffold
13. `TBD` - Expanded native and Web/config contract tests
14. `TBD` - WLED HTTP action
15. `TBD` - WLED interface kill switch and disabled default
16. `TBD` - Incoming MIDI Web page chunking fix
17. `TBD` - Web hardware test page

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

### `TBD` - Native unit test scaffold
- Added a PlatformIO `native` environment backed by a project-local `native_host` board definition for host-side Unity tests.
- Extracted dependency-free core helpers for `map2` and incoming MIDI trigger matching into `src/PedalinoCoreLogic.h`, and wired the existing firmware paths through those helpers.
- Added native Unity coverage for `map2` endpoint/rounding/zero-width behavior and incoming MIDI trigger matching for CC/PC, Any/Exact values, Any channel, wrong channel/number, and unsupported message types.
- Wired native unit tests into build and CodeQL CI, and added local validation plus native unit tests to the release workflow before firmware packaging.
- Compatibility note: intended firmware behavior and config format are unchanged; this is a testability and CI-gating change.
- Validation: `python -m unittest scripts.test_validate_config_surfaces`, `python scripts/validate_config_surfaces.py`, `$env:TMPDIR = "C:\tmp"; pio test -e native`, and `$env:TMPDIR = "C:\tmp"; pio run -e lilygo-t-display-s3`.

### `TBD` - Expanded native and Web/config contract tests
- Expanded host-side native Unity coverage for analog response mapping, display overlay label selection, slot-state tag decisions, incoming MIDI dispatch order and bank scope handling, RGB LED order swaps, chunked Web UI page trimming, and tap-tempo averaging/reset behavior.
- Added Python contract coverage for `/incoming-actions` route/form fields, duplicate-bank action wiring, sequence-name UI/config contracts, legacy incoming-action import warnings, `SequenceNames` schema/runtime limits, and current plus legacy `IncomingTriggers` fixture support.
- Wired the new Web/config contract tests into build, CodeQL, and release workflows alongside the existing local validation tests.
- Compatibility note: intended firmware behavior and config format are unchanged; production paths now delegate more small decisions to dependency-free helpers so they can be regression-tested on the native host target.
- Validation: `python -m unittest scripts.test_validate_config_surfaces scripts.test_web_config_contracts`, `python scripts/validate_config_surfaces.py`, `$env:TMPDIR = "C:\tmp"; pio test -e native`, and `$env:TMPDIR = "C:\tmp"; pio run -e lilygo-t-display-s3`.

### `TBD` - WLED HTTP action
- Added first-class `WLED` actions for normal actions, sequence steps, and incoming MIDI target actions.
- Added global `WLEDAddress` configuration in Options, exported JSON, schema, and NVS/SPIFFS globals. The value is a host/IP with optional port; firmware sends to `http://<WLEDAddress>/json/state`.
- Added queued best-effort WLED HTTP delivery from the low-priority Wi-Fi loop, with a small fixed queue, 500 ms HTTP timeout, no retry/persistent queue, and debug drops when the address is empty or invalid.
- Added readable exported/imported `WLED` objects with `Command`, `Value`, `Speed`, `Intensity`, and `Color` fields while keeping runtime storage mapped into existing compact byte/color fields.
- Supported WLED commands: `Power`, `Preset`, `Brightness`, `Solid Color`, and `Effect`.
- Compatibility note: existing non-WLED action, sequence, and incoming-trigger serialization remains unchanged. WLED actions require Wi-Fi and one configured global WLED target; analog/jog continuous WLED output is intentionally not supported in this first pass.
- Validation: `python -m unittest scripts.test_validate_config_surfaces scripts.test_web_config_contracts`, `python scripts/validate_config_surfaces.py`, `$env:TMPDIR = "$PWD\.tmp"; pio test -e native`, and `$env:TMPDIR = "$PWD\.tmp"; pio run -e lilygo-t-display-s3`.

### `TBD` - WLED interface kill switch and disabled default
- Added a seventh `WLED` row to the Interfaces tab with a single `Enabled` switch backed by the interface `Out` state.
- WLED actions now check the WLED interface state before enqueueing or sending HTTP requests, so disabling WLED blocks normal action, sequence, and incoming-trigger WLED output paths.
- Clearing the WLED interface also clears any pending WLED HTTP queue entries, preventing stale WLED requests from firing later after the interface is re-enabled.
- Older six-interface profile/config data remains compatible: missing WLED interface entries default to disabled, while explicit seven-interface configs preserve their saved WLED state.
- Tightened the OSC `/interface` index clamp to stay within the expanded interfaces array.
- Compatibility note: WLED output is disabled by default. Use Interfaces -> WLED -> Enabled to allow WLED action side effects when the integration is in use.
- Validation: `python -m unittest scripts.test_validate_config_surfaces scripts.test_web_config_contracts`, `python scripts/validate_config_surfaces.py`, `pio test -e native`, and `pio run -e lilygo-t-display-s3`.

### `TBD` - Incoming MIDI Web page chunking fix
- Added extra chunk-trim checkpoints inside the Incoming MIDI Actions editor after trigger headers, LED action fields, and WLED action fields.
- This keeps the generated `/incoming-actions` HTML chunks below the reserved 8192-byte page buffer more consistently and avoids the serial `Memory fragmentation warning: webpage memory allocation ... greater then 8192 bytes reserved` message seen when editing WLED incoming actions.
- Compatibility note: no config format or runtime MIDI behavior changed; this only changes web-page chunk generation.
- Validation: `python -m unittest scripts.test_validate_config_surfaces scripts.test_web_config_contracts`, `python scripts/validate_config_surfaces.py`, and `pio run -e lilygo-t-display-s3`.

### `TBD` - Web hardware test page
- Added a WebSocket-first `/hardware` page for LilyGO T-Display S3 builds with six virtual controls mapped to fixed Controls `1..6` and a read-only 2x3 display-state preview.
- Enabled `WEBSOCKET` only for the `lilygo-t-display-s3` PlatformIO environment, leaving other board build flags unchanged.
- Added runtime helpers that resolve a virtual control to its primary single momentary-style pedal/button mapping and queue `Pressed` / `Released` events for dispatch from the normal controller loop.
- Added compact `hardware` EventSource updates for current bank label, virtual button labels/enabled state, and visible slot labels/state; state is sent when the snapshot changes while a hardware page WebSocket client is connected.
- Hardened the live test transport by applying the configured web credentials to `/ws` and `/events`, bounding WebSocket message parsing, disabling controls without single-press events or unavailable button slots, tracking browser-held virtual controls by WebSocket client, and queueing release events on that client's disconnect/error.
- Added host-side coverage for virtual-control mapping and display-label fallback helpers, plus Web/config contract coverage for `/hardware` route and browser command/event wiring.
- Compatibility note: saved configuration format is unchanged. Unsupported virtual controls, including unmapped controls, simultaneous controls, non-momentary pedal modes, controls without single-press events, and unavailable button slots are surfaced as disabled browser buttons instead of adding a separate action path. If an HTTP username is configured with a blank HTTP password, the hardware test page is unavailable until a password is set because the async WebSocket/EventSource handlers cannot enforce that credential shape. Changing the Web UI username/password now persists the credentials immediately and restarts the device so live transport auth is rebuilt.
- Size note for `lilygo-t-display-s3`: baseline firmware was `2,408,105` bytes flash / `107,176` bytes RAM; pre-merge feature build was `2,436,981` bytes flash / `107,504` bytes RAM, leaving `1,101,963` bytes free in the `3,538,944` byte OTA app slot.
- Validation: `python -m unittest scripts.test_validate_config_surfaces scripts.test_web_config_contracts`, `python scripts/validate_config_surfaces.py`, `$env:TMPDIR = "C:\tmp"; pio test -e native`, `$env:TMPDIR = "C:\tmp"; pio run -e lilygo-t-display-s3 -t buildfs`, and `$env:TMPDIR = "C:\tmp"; pio run -e lilygo-t-display-s3`.


## Validation
- Latest verified build: `$env:TMPDIR = "C:\tmp"; pio run -e lilygo-t-display-s3` (success, 2026-06-03, merged `custom-open-source` into Web hardware test page worktree; RAM `107,600` bytes, flash `2,455,169` bytes, `1,083,775` bytes free in the OTA app slot).
- Latest filesystem build: `$env:TMPDIR = "C:\tmp"; pio run -e lilygo-t-display-s3 -t buildfs` (success, 2026-06-03, merged Web hardware test page worktree).
- Latest native test run: `$env:TMPDIR = "C:\tmp"; pio test -e native` (31 Unity tests passed, 2026-06-03).
- Latest local Python validation: `python -m unittest scripts.test_validate_config_surfaces scripts.test_web_config_contracts` (23 tests passed, 2026-06-03) and `python scripts/validate_config_surfaces.py` (success, 2026-06-03).
- Latest device check: existing grouped incoming MIDI rules migrated into `Global` bank `0` and no longer caused a reboot loop on LilyGO T-Display S3.
