# PedalinoMini Agent Guide

This file is the root-level agent context for `PedalinoMini`. It is based on the project context previously kept in `docs/AI_CONTEXT.md`.

## Startup Checklist
- Before starting substantive work, review the release notes to pick up the latest fork context:
  - `FORK-RELEASE-NOTES.md` at the repository root is the current fork-specific source of truth.
- Review relevant feature docs under `docs/features/` for the area being changed. Not all the features have been implemented yet.
- Review the latest project snapshot under `docs/project_snapshots/` when it is relevant to the current task but bear in mind the context might be out of date

## Project Snapshot
- `PedalinoMini` is ESP32/ESP32-S3 firmware for DIY MIDI/HID controllers such as footswitches, expression pedals, jog wheels, and similar inputs.
- The primary modern target is ESP32-S3, especially `lilygo-t-display-s3`, while legacy ESP32 targets are still supported.
- Device behavior follows this chain:
  `Pedals (physical inputs) -> Controls (logical mapping) -> Actions (messages/commands) -> MIDI/HID/OSC outputs`
- The on-device web UI configures most behavior and persists configuration to SPIFFS/JSON plus some globals/NVS storage.

## Build And Flash
- Core config file: `platformio.ini`
- Typical build:
  - `pio run -e lilygo-t-display-s3`
- Windows PowerShell fallback when `pio` is not on `PATH`:
  - `$pioExe = "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe"`
  - `& $pioExe run -e lilygo-t-display-s3`
- Typical upload:
  - `pio run -e lilygo-t-display-s3 -t upload`
- Windows PowerShell upload fallback:
  - `$pioExe = "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe"`
  - `& $pioExe run -e lilygo-t-display-s3 -t upload`
- Common feature toggles live in `platformio.ini`, for example:
  - `NOWIFI`
  - `NOBLE`
  - `SMARTCONFIG`
  - `WPS`
  - `NVS`
  - `DIAGNOSTIC`

## Runtime Architecture
- Entry point: `src/PedalinoMini.cpp`
- `setup()`:
  - Initializes hardware, storage, display, LEDs, connectivity, and profile/config state.
  - Handles BOOT-button hold timing for normal, BLE, Wi-Fi, AP, ladder-config, and factory-reset paths.
  - Starts USB MIDI, DIN MIDI, BLE MIDI, and Wi-Fi-related services.
  - Spawns two FreeRTOS tasks, `loop0` and `loop1`, pinned across cores.
- `loop()`:
  - Runs high-priority scan and dispatch via `controller_run()`, MIDI clock/MTC work, and some LED/display effects.
- `loop1()`:
  - Polls incoming MIDI from enabled interfaces.
- `loop0()`:
  - Handles lower-priority Wi-Fi, web, config, and housekeeping work.

## Core Data Model
Defined mainly in `src/Pedalino.h`:
- Global limits/constants:
  - `PROFILES=3`
  - `BANKS=21` where bank `0` is global
  - `CONTROLS=100`
  - `SEQUENCES=20`
  - `STEPS=10`
- `struct pedal`:
  - Physical input mode and behavior such as momentary, latch, analog, jog, ladder, thresholds, and calibration.
- `struct control`:
  - Logical input definition, including single-switch and simultaneous-combo mappings plus LED linkage.
- `struct action`:
  - Event-triggered output behavior, stored as linked lists per bank.
- `struct message`:
  - Sequence step payload.
- Important globals:
  - `pedals[]`
  - `controls[]`
  - `actions[BANKS]`
  - `sequences[SEQUENCES][STEPS]`
  - `interfaces[]`

## Key Files By Intent
- Main boot/runtime flow:
  - `src/PedalinoMini.cpp`
- Input scanning, action dispatch, LED state logic:
  - `src/Controller.h`
- Config serialization/deserialization and SPIFFS/NVS helpers:
  - `src/Config.h`
  - Schema: `data/schema.json`
- Web configuration UI routes and HTML generation:
  - `src/WebConfigAsync.h`
- Connectivity interfaces:
  - BLE MIDI: `src/BLEMidiIn.h`, `src/BLEMidiOut.h`
  - USB MIDI: `src/USBMidiIn.h`, `src/USBMidiOut.h`
  - DIN MIDI: `src/SerialMidiIn.h`, `src/SerialMidiOut.h`
  - RTP/ip/UDP MIDI: `src/UdpMidiIn.h`, `src/UdpMidiOut.h`
- Board pin maps and feature constants:
  - `src/Pedalino.h`

## Practical Editing Notes
- Verify source-of-truth doc paths before editing; in this fork some docs have moved between `docs/` and the repository root.
- Prefer small, targeted edits. The codebase relies heavily on globals and compile-time flags.
- Keep behavior consistent across these linked surfaces:
  1. Runtime enums/constants in `Pedalino.h`
  2. String mappings in `Config.h` such as `ActionStringToEnum` and `ActionEnumToString`
  3. Web UI options/forms in `WebConfigAsync.h`
  4. JSON schema validation in `data/schema.json`
- When adding new action or event types, update all four of the surfaces above.
- Be careful with board-specific pin definitions and ESP32/ESP32-S3 strapping pins.
- Preserve the memory-conscious patterns already in use, including chunked web page generation and PSRAM-aware JSON allocation.
- After any feature or behavior change, update the main project release-notes file in its current source-of-truth location during the same work session.
  - Include affected commits or the current placeholder section, user-visible changes, compatibility notes, and validation/build status.
- After any fork-specific code or behavior change, update `FORK-RELEASE-NOTES.md` in the same work session.
  - Do not use an `Unreleased` section there.
  - Instead, add a new commit section with a placeholder hash such as `TBD` and replace it with the real commit hash later.

## Fresh-Agent Sufficiency
- This file should be enough for a fresh agent to start productive work, especially when combined with:
  - `FORK-RELEASE-NOTES.md`
  - relevant feature docs in `docs/features/`
  - any current snapshot in `docs/project_snapshots/`
- The highest-value context for a new agent is:
  - the runtime/control/action architecture
  - the linked edit surfaces (`Pedalino.h`, `Config.h`, `WebConfigAsync.h`, `data/schema.json`)
  - the current release notes and fork notes
  - the current target hardware (`lilygo-t-display-s3`)
- If anything else is added later, the most useful additions would be:
  - a short source-of-truth map for docs that have moved
  - a brief list of currently active feature areas under development
  - a compact verification checklist for common firmware changes

## Hardware Context
- Current user-relevant setup: LilyGO T-Display S3 with 6 momentary footswitches.
- For this setup, most changes usually live in:
  - Pedal mode and press-behavior defaults
  - Controls mapping
  - Action definitions and bank logic
  - MIDI routing and interface enablement

## Non-Goals And Caution
- Do not rewrite large subsystems unless the task clearly requires it.
- Avoid changing partitions, boot logic, or board pin maps unless the task explicitly calls for it.
- Keep backward compatibility with existing config files whenever possible.
