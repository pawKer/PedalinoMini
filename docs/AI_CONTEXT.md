# PedalinoMini AI Context

## Project Snapshot
- `PedalinoMini` is ESP32/ESP32-S3 firmware for DIY MIDI/HID controllers (footswitches, expression pedals, jog wheels, etc.).
- Primary modern target is ESP32-S3 (notably `lilygo-t-display-s3`), with legacy ESP32 targets still supported.
- Device behavior is driven by the chain:
  `Pedals (physical inputs) -> Controls (logical mapping) -> Actions (messages/commands) -> MIDI/HID/OSC outputs`.
- Web UI on device configures almost everything and persists configuration to SPIFFS/JSON (and some globals/NVS).

## Build + Flash (PlatformIO)
- Core config file: `platformio.ini`
- Typical build:
  - `pio run -e lilygo-t-display-s3`
- Typical upload:
  - `pio run -e lilygo-t-display-s3 -t upload`
- Common feature toggles are compile flags in `platformio.ini`, e.g.:
  - `NOWIFI` (disable Wi-Fi)
  - `NOBLE` (disable BLE)
  - `SMARTCONFIG`, `WPS`, `NVS`, `DIAGNOSTIC`

## Runtime Architecture (High Level)
- Entry point: `src/PedalinoMini.cpp`
- `setup()`:
  - Initializes hardware, storage, display, LEDs, connectivity, and profile/config state.
  - Handles boot-mode selection via BOOT button hold timing (normal/BLE/Wi-Fi/AP/ladder/factory reset paths).
  - Starts interfaces (USB MIDI, DIN MIDI, BLE MIDI, Wi-Fi stack).
  - Spawns two FreeRTOS tasks (`loop0`, `loop1`) pinned across cores.
- `loop()` (Core 1 focus):
  - Runs high-priority scan/dispatch (`controller_run()`), MIDI clock/MTC loop, and some LED/display effects.
- `loop1()`:
  - Polls incoming MIDI from enabled interfaces.
- `loop0()`:
  - Lower-priority services (Wi-Fi/web/config/housekeeping, depending on build flags and mode).

## Core Data Model
Defined mostly in `src/Pedalino.h`:
- Global limits/constants:
  - `PROFILES=3`, `BANKS=21` (bank 0 is global), `CONTROLS=100`, `SEQUENCES=20`, `STEPS=10`.
- `struct pedal`:
  - Physical input mode and behavior (momentary/latch/analog/jog/ladder/etc.), thresholds/calibration.
- `struct control`:
  - Logical input definition (single switch or simultaneous combo), default LED mapping.
- `struct action`:
  - Event-triggered output behavior (MIDI/sequence/special actions/keyboard/OSC), linked list per bank.
- `struct message`:
  - Sequence step payload.
- Key globals:
  - `pedals[]`, `controls[]`, `actions[BANKS]`, `sequences[SEQUENCES][STEPS]`, `interfaces[]`.

## Key Files To Edit By Intent
- Main boot/runtime flow:
  - `src/PedalinoMini.cpp`
- Input scanning, action dispatch, LED state logic:
  - `src/Controller.h`
- Config serialization/deserialization (SPIFFS JSON/NVS helpers):
  - `src/Config.h`
  - JSON schema: `data/schema.json`
- Web configuration UI routes/HTML generation:
  - `src/WebConfigAsync.h`
- Connectivity interfaces:
  - BLE MIDI: `src/BLEMidiIn.h`, `src/BLEMidiOut.h`
  - USB MIDI: `src/USBMidiIn.h`, `src/USBMidiOut.h`
  - DIN serial MIDI: `src/SerialMidiIn.h`, `src/SerialMidiOut.h`
  - RTP/ip/UDP MIDI: `src/UdpMidiIn.h`, `src/UdpMidiOut.h`
- Board pin maps + feature constants:
  - `src/Pedalino.h`

## Practical Notes For AI Edits
- Prefer small, targeted edits. This codebase relies heavily on globals and compile-time flags.
- Keep behavior consistent across:
  1. Runtime enums/constants in `Pedalino.h`
  2. String mapping in `Config.h` (`ActionStringToEnum` / `ActionEnumToString`)
  3. Web UI options/forms in `WebConfigAsync.h`
  4. JSON schema validation in `data/schema.json`
- When adding new action/event types, update all four surfaces above.
- Be careful with board-specific pin definitions and strapping pins (ESP32/ESP32-S3 boot constraints).
- Preserve existing memory-conscious patterns (chunked web page building, PSRAM-aware JSON allocation).

## Hardware Context (Current User Setup)
- User-relevant setup: LilyGO T-Display S3 with 6 momentary footswitches.
- For this style of setup, most changes usually live in:
  - Pedal mode/press behavior defaults
  - Controls mapping
  - Action definitions and bank logic
  - MIDI routing/interface enablement

## Non-Goals / Caution
- Do not rewrite large subsystems without need; many features are interconnected through shared globals.
- Avoid changing partition files, boot logic, or board pin maps unless task explicitly requires it.
- Keep backward compatibility with existing config files whenever possible.
