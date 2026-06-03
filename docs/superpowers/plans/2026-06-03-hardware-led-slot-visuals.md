# Hardware LED Slot Visuals Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Extend `/hardware` so the browser preview shows configured slot border colors and the current mapped LED color for each virtual control.

**Architecture:** Keep this as a read-only extension of the existing `/hardware` state payload. Add compact JSON fields in `hardware_state_json()`, render them with the existing page JavaScript, and add one dependency-free RGB helper so cached hardware-order LED colors can be converted back to browser CSS RGB.

**Tech Stack:** C++/Arduino firmware, ESPAsyncWebServer/WebSocket/EventSource, PlatformIO native Unity tests, Python contract tests.

---

### Task 1: Contract Tests For `/hardware` State And Rendering

**Files:**
- Modify: `scripts/test_web_config_contracts.py`

- [ ] **Step 1: Write failing contract tests**

Add assertions that `hardware_state_json()` emits `borderColor`, `led`, `ledColor`, and `ledActive`, and that `hardwareApplyState()` updates the slot border/background and a button LED element.

- [ ] **Step 2: Run test to verify it fails**

Run: `python -B -m unittest scripts.test_web_config_contracts.WebConfigContractTest`

Expected: FAIL because the new JSON keys and JavaScript snippets do not exist yet.

### Task 2: Native RGB Helper Test

**Files:**
- Modify: `test/test_core_logic/test_core_logic.cpp`
- Modify: `src/PedalinoCoreLogic.h`

- [ ] **Step 1: Write failing native test**

Add `test_unswap_rgb_order_restores_browser_rgb_from_hardware_cache()` beside the existing RGB order tests.

- [ ] **Step 2: Run test to verify it fails**

Run: `$env:TMPDIR = "C:\tmp"; pio test -e native`

Expected: FAIL because `pedalino::unswap_rgb_order()` is not defined.

- [ ] **Step 3: Implement minimal helper**

Add `pedalino::unswap_rgb_order()` that reverses `swap_rgb_order()` for each supported order.

### Task 3: Hardware State Payload And Page Rendering

**Files:**
- Modify: `src/WebConfigAsync.h`

- [ ] **Step 1: Add color formatting helpers**

Add a small `hardware_append_json_color()` helper and a `hardware_led_css_color()` helper that converts `lastLedColor[currentBank][l]` back to CSS RGB using `unswap_rgb_order()`.

- [ ] **Step 2: Extend button JSON**

For each of the six hardware buttons, include its mapped LED index, CSS LED color, and whether the LED is visibly active.

- [ ] **Step 3: Extend slot JSON**

For each of the six display slots, include the configured `slotBorderColor[s]` as a CSS hex string.

- [ ] **Step 4: Update `/hardware` markup and JavaScript**

Add a small LED indicator span inside each button and update `hardwareApplyState()` to set indicator color/visibility plus slot border/background from the new state fields.

### Task 4: Docs, Build, And Commit

**Files:**
- Modify: `FORK-RELEASE-NOTES.md`

- [ ] **Step 1: Update release notes**

Add a new `TBD` section describing the `/hardware` visual-state enhancement, compatibility note, and validation commands.

- [ ] **Step 2: Run verification**

Run:
- `python -B -m unittest scripts.test_validate_config_surfaces scripts.test_web_config_contracts`
- `python -B scripts/validate_config_surfaces.py`
- `$env:TMPDIR = "C:\tmp"; pio test -e native`
- `$env:TMPDIR = "C:\tmp"; pio run -e lilygo-t-display-s3`
- `$env:TMPDIR = "C:\tmp"; pio run -e lilygo-t-display-s3 -t buildfs`

- [ ] **Step 3: Commit**

Commit all changed files with message `feat: show hardware LED and slot visuals`.
