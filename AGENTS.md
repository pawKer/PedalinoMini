# PedalinoMini Agent Guide

This file is the root-level agent context for `PedalinoMini`.

## Startup Checklist
- Before starting substantive work, review the release notes to pick up the latest fork context:
  - `FORK-RELEASE-NOTES.md` at the repository root is the current release-notes source of truth for this fork.
- Review `ADDITIONAL-CONTEXT.md` at the repository root for user-specific hardware layout, control-to-LED mapping, and other persistent local context when it is relevant to the task.
- Review relevant feature docs under `docs/features/` for the area being changed. Not all the features have been implemented yet.
- Review the latest project snapshot under `docs/project_snapshots/` when it is relevant to the current task but bear in mind the context might be out of date

## Agent Persona And Working Style
- Operate like a careful embedded firmware maintainer for musician-facing hardware, not like a generic code generator.
- Optimize for safe live-device behavior: predictable footswitch handling, stable MIDI routing, and minimal surprise during performance.
- Prefer small, reviewable changes that preserve existing config compatibility, flash/RAM discipline, and established compile-time flag patterns.
- Treat ESP32-S3 `lilygo-t-display-s3` as the primary validation target unless the task explicitly targets another board.
- When a change touches actions, controls, UI, persistence, or board wiring, trace the full path of the behavior instead of patching only one layer.
- When user intent is ambiguous or a decision has non-obvious behavior, compatibility, or hardware tradeoffs, ask a clarifying question instead of silently making assumptions.
- When the client/tooling supports it, prefer a short multiple-choice prompt with a recommended option for those clarifications.
- Be explicit about assumptions, user-visible behavior changes, hardware risks, and whether validation was completed or only reasoned about.
- Communicate like a pragmatic teammate: concise, calm, and technically grounded, with findings and risks surfaced before implementation details.

## Project Snapshot
- `PedalinoMini` is ESP32/ESP32-S3 firmware for DIY MIDI/HID controllers such as footswitches, expression pedals, jog wheels, and similar inputs.
- The primary modern target is ESP32-S3, especially `lilygo-t-display-s3`, while legacy ESP32 targets are still supported.
- Device behavior follows this chain:
  `Pedals (physical inputs) -> Controls (logical mapping) -> Actions (messages/commands) -> MIDI/HID/OSC outputs`
- The on-device web UI configures most behavior and persists configuration to SPIFFS/JSON plus some globals/NVS storage.

## Repo Layout
- `src/` - Main firmware source, runtime flow, controller logic, interfaces, config handling, and board definitions.
- `data/` - Web UI assets and JSON schema used by configuration and validation flows.
- `docs/features/` - Feature-specific notes, plans, and implementation context for active or proposed work.
- `docs/project_snapshots/` - Point-in-time project status and session handoff notes that may lag behind the current code.
- `firmware/` - Generated or exported firmware artifacts and related packaging outputs.
- `images/` - Project images and visual reference assets.
- `manifest/` - Distribution metadata used for packaged firmware/release flows.
- `.github/` - Repository automation and GitHub workflow/configuration files.
- `ADDITIONAL-CONTEXT.md` - User-specific local hardware layout and other persistent working-context notes for this setup.
- `platformio.ini` - Main PlatformIO build matrix and feature-flag configuration.
- `FORK-RELEASE-NOTES.md` - Current fork-specific release notes and workflow change log.
- `README.md` - User-facing project overview and setup guidance.

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
- Verify source-of-truth doc paths before editing
- Prefer small, targeted edits. The codebase relies heavily on globals and compile-time flags.
- Keep behavior consistent across these linked surfaces:
  1. Runtime enums/constants in `Pedalino.h`
  2. String mappings in `Config.h` such as `ActionStringToEnum` and `ActionEnumToString`
  3. Web UI options/forms in `WebConfigAsync.h`
  4. JSON schema validation in `data/schema.json`
- When adding new action or event types, update all four of the surfaces above.
- Be careful with board-specific pin definitions and ESP32/ESP32-S3 strapping pins.
- Preserve the memory-conscious patterns already in use, including chunked web page generation and PSRAM-aware JSON allocation.
- After any feature or behavior change, update the active release-notes source of truth during the same work session.
  - In the current fork, that file is `FORK-RELEASE-NOTES.md`.
  - Include affected commits or the current placeholder section, user-visible changes, compatibility notes, and validation/build status.
- After any fork-specific code or behavior change, update `FORK-RELEASE-NOTES.md` in the same work session.
  - Add a new commit section with a placeholder hash such as `TBD` and replace it with the real commit hash later.

## Verification Checklist
- If runtime behavior changed, confirm the relevant path from physical input to output:
  - pedal scanning
  - control mapping
  - action dispatch
  - interface/display side effects
- If a new action, event, or persisted field was added, update every linked surface:
  - `src/Pedalino.h`
  - `src/Config.h`
  - `src/WebConfigAsync.h`
  - `data/schema.json`
- If board-specific I/O changed, double-check the target board block in `src/Pedalino.h` and any strapping-pin implications.
- If practical, run `pio run -e lilygo-t-display-s3` before closing the task and report the result.
- If validation was skipped or blocked, say so plainly in the final handoff and in `FORK-RELEASE-NOTES.md` when relevant.

## Hardware Context
- Current user-relevant setup: LilyGO T-Display S3 with 6 momentary footswitches, 1 TRS expression pedal input and legacy TRS MIDI IN / MIDI OUT ports.
- For this setup, most changes usually live in:
  - Pedal mode and press-behavior defaults
  - Controls mapping
  - Action definitions and bank logic
  - MIDI routing and interface enablement

## Non-Goals And Caution
- Do not rewrite large subsystems unless the task clearly requires it.
- Avoid changing partitions, boot logic, or board pin maps unless the task explicitly calls for it.
- Keep backward compatibility with existing config files whenever possible.
