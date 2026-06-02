# Web Hardware Test Page Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a WebSocket-first `/hardware` page for testing the six main controls from a browser while mirroring the visible 2x3 display state.

**Architecture:** Keep the feature scoped to WebConfig/runtime display helpers instead of adding a parallel action runner. WebSocket commands resolve fixed Controls 1-6 to their primary momentary-style pedal/button mapping, then inject `Pressed` and `Released` events into the existing controller handler. A compact hardware-state helper builds the labels/states used by the page and SSE payload.

**Tech Stack:** ESP32 Arduino, ESPAsyncWebServer WebSocket/EventSource, PlatformIO `lilygo-t-display-s3`, native Unity tests for dependency-free helper logic, Python Web/config contract tests.

---

### Task 1: Enable WebSocket For LilyGO T-Display S3 Only

**Files:**
- Modify: `platformio.ini`
- Test: firmware size check with `pio run -e lilygo-t-display-s3`

- [ ] **Step 1: Record the current size baseline**

Run:

```powershell
$env:TMPDIR = "C:\tmp"
$pioExe = "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe"
& $pioExe run -e lilygo-t-display-s3
```

Expected baseline:

```text
RAM:   [===       ]  32.7% (used 107176 bytes from 327680 bytes)
Flash: [=======   ]  68.0% (used 2408105 bytes from 3538944 bytes)
```

- [ ] **Step 2: Enable WebSocket only in the S3 build flags**

In `[common]`, leave `; -D WEBSOCKET` commented. In `build_flags_lilygo-t-display-s3`, add:

```ini
	-D WEBSOCKET
```

Do not add WebSocket to non-S3 environments in this task.

- [ ] **Step 3: Rebuild and compare size**

Run:

```powershell
$env:TMPDIR = "C:\tmp"
$pioExe = "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe"
& $pioExe run -e lilygo-t-display-s3
```

Expected: build succeeds and the new flash usage leaves meaningful OTA app headroom. If the WebSocket-only delta is unexpectedly large, stop and report before adding the page.

- [ ] **Step 4: Commit**

```powershell
git add platformio.ini
git commit -m "build: enable websocket for s3 hardware testing"
```

### Task 2: Add Dependency-Free Hardware State Helpers

**Files:**
- Create or modify: `src/PedalinoCoreLogic.h`
- Modify: `test/test_core_logic/test_core_logic.cpp`

- [ ] **Step 1: Write failing native tests**

Add tests that describe the intended helper behavior:

```cpp
void test_hardware_control_mapping_accepts_single_momentary_primary_mapping() {
  HardwareControlMapping mapping = hardware_control_mapping_for(
      0, 0, PEDALS, LADDER_STEPS, PED_MOMENTARY1);
  TEST_ASSERT_TRUE(mapping.supported);
  TEST_ASSERT_EQUAL_UINT8(0, mapping.pedal);
  TEST_ASSERT_EQUAL_UINT8(0, mapping.button);
  TEST_ASSERT_EQUAL_STRING("", mapping.reason);
}

void test_hardware_control_mapping_rejects_simultaneous_control() {
  HardwareControlMapping mapping = hardware_control_mapping_for(
      0, 0, 1, 0, PED_MOMENTARY1);
  TEST_ASSERT_FALSE(mapping.supported);
  TEST_ASSERT_EQUAL_STRING("Simultaneous control", mapping.reason);
}

void test_hardware_control_mapping_rejects_non_momentary_mode() {
  HardwareControlMapping mapping = hardware_control_mapping_for(
      0, 0, PEDALS, LADDER_STEPS, PED_ANALOG);
  TEST_ASSERT_FALSE(mapping.supported);
  TEST_ASSERT_EQUAL_STRING("Unsupported pedal mode", mapping.reason);
}

void test_hardware_display_label_prefers_active_tag_then_fallback() {
  TEST_ASSERT_EQUAL_STRING("PLAY", hardware_display_label("STOP", "PLAY", true, "Control 1").c_str());
  TEST_ASSERT_EQUAL_STRING("STOP", hardware_display_label("STOP", "PLAY", false, "Control 1").c_str());
  TEST_ASSERT_EQUAL_STRING("Control 1", hardware_display_label("", "", false, "Control 1").c_str());
}
```

Register the tests in `main()`.

- [ ] **Step 2: Verify the tests fail**

Run:

```powershell
$env:TMPDIR = "C:\tmp"
$pioExe = "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe"
& $pioExe test -e native
```

Expected: compile failure because `HardwareControlMapping`, `hardware_control_mapping_for`, and `hardware_display_label` do not exist yet.

- [ ] **Step 3: Implement the helpers**

Add dependency-free helpers in `PedalinoCoreLogic.h`:

```cpp
struct HardwareControlMapping {
  bool supported;
  uint8_t pedal;
  uint8_t button;
  const char* reason;
};

inline bool hardware_pedal_mode_is_virtual_pressable(uint8_t pedalMode) {
  return pedalMode == PED_MOMENTARY1 ||
         pedalMode == PED_MOMENTARY2 ||
         pedalMode == PED_MOMENTARY3 ||
         pedalMode == PED_LADDER ||
         pedalMode == PED_ANALOG_MOMENTARY;
}

inline HardwareControlMapping hardware_control_mapping_for(
    uint8_t pedal1,
    uint8_t button1,
    uint8_t pedal2,
    uint8_t button2,
    uint8_t pedalMode) {
  if (pedal1 >= PEDALS || button1 >= LADDER_STEPS) {
    return {false, 0, 0, "Unmapped control"};
  }
  if (pedal2 != PEDALS || button2 != LADDER_STEPS) {
    return {false, 0, 0, "Simultaneous control"};
  }
  if (!hardware_pedal_mode_is_virtual_pressable(pedalMode)) {
    return {false, 0, 0, "Unsupported pedal mode"};
  }
  return {true, pedal1, button1, ""};
}

inline String hardware_display_label(
    const String& tag0,
    const String& tag1,
    bool active,
    const String& fallback) {
  const String& selected = active ? tag1 : tag0;
  return selected.length() > 0 ? selected : fallback;
}
```

- [ ] **Step 4: Verify native tests pass**

Run:

```powershell
$env:TMPDIR = "C:\tmp"
$pioExe = "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe"
& $pioExe test -e native
```

Expected: all native tests pass.

- [ ] **Step 5: Commit**

```powershell
git add src/PedalinoCoreLogic.h test/test_core_logic/test_core_logic.cpp
git commit -m "test: cover hardware page helper logic"
```

### Task 3: Add Runtime Injection And Hardware State Payload

**Files:**
- Modify: `src/Controller.h`
- Modify: `src/DisplayTFT.h`
- Modify: `src/WebConfigAsync.h`

- [ ] **Step 1: Add control injection helper**

In `Controller.h`, add:

```cpp
inline bool controller_virtual_control_event(byte controlIndex, uint8_t eventType, String* reason = nullptr) {
  if (controlIndex >= CONTROLS) {
    if (reason) *reason = "Invalid control";
    return false;
  }

  HardwareControlMapping mapping = hardware_control_mapping_for(
      controls[controlIndex].pedal1,
      controls[controlIndex].button1,
      controls[controlIndex].pedal2,
      controls[controlIndex].button2,
      controls[controlIndex].pedal1 < PEDALS ? pedals[controls[controlIndex].pedal1].mode : PED_NONE);

  if (!mapping.supported) {
    if (reason) *reason = mapping.reason;
    return false;
  }

  AceButton* button = pedals[mapping.pedal].button[mapping.button];
  if (button == nullptr) {
    if (reason) *reason = "Pedal button unavailable";
    return false;
  }

  controller_event_handler_button(button, eventType, 0);
  return true;
}
```

- [ ] **Step 2: Add compact hardware state JSON builder**

In `WebConfigAsync.h`, add a helper guarded by `#ifdef WEBSOCKET`:

```cpp
String hardware_state_json() {
  JsonDocument doc;
  doc["bank"] = currentBank;
  doc["bankName"] = banknames[currentBank];

  JsonArray buttons = doc["buttons"].to<JsonArray>();
  for (byte i = 0; i < 6; i++) {
    JsonObject item = buttons.add<JsonObject>();
    String reason;
    HardwareControlMapping mapping = hardware_control_mapping_for(
        controls[i].pedal1,
        controls[i].button1,
        controls[i].pedal2,
        controls[i].button2,
        controls[i].pedal1 < PEDALS ? pedals[controls[i].pedal1].mode : PED_NONE);
    item["id"] = i + 1;
    item["label"] = String("Control ") + String(i + 1);
    item["enabled"] = mapping.supported;
    item["reason"] = mapping.supported ? "" : mapping.reason;
  }

  JsonArray slots = doc["slots"].to<JsonArray>();
  for (byte s = 0; s < SLOTS && s < 6; s++) {
    JsonObject item = slots.add<JsonObject>();
    bool active = slotDisplayInitialized[currentBank][s] ? slotDisplayState[currentBank][s] : false;
    item["id"] = s + 1;
    item["label"] = String("S") + String(s + 1);
    item["active"] = active;
  }

  String json;
  serializeJson(doc, json);
  return json;
}
```

Then refine labels by reusing the display slot/action label selection already present in `DisplayTFT.h`, keeping the payload compact.

- [ ] **Step 3: Publish hardware state**

Add:

```cpp
void hardware_send_state() {
#ifdef WEBSOCKET
  if (WiFi.isConnected() || WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
    String json = hardware_state_json();
    events.send(json.c_str(), "hardware");
  }
#endif
}
```

Call `hardware_send_state()` after WebSocket virtual press/release handling, after bank changes inside the WebSocket handler, and from visible display update paths after slot state changes are rendered.

- [ ] **Step 4: Build for S3**

Run:

```powershell
$env:TMPDIR = "C:\tmp"
$pioExe = "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe"
& $pioExe run -e lilygo-t-display-s3
```

Expected: build succeeds.

- [ ] **Step 5: Commit**

```powershell
git add src/Controller.h src/DisplayTFT.h src/WebConfigAsync.h
git commit -m "feat: add hardware test runtime state"
```

### Task 4: Add `/hardware` Page And WebSocket Commands

**Files:**
- Modify: `src/WebConfigAsync.h`
- Modify: `scripts/test_web_config_contracts.py`

- [ ] **Step 1: Add failing Python contract tests**

Add tests that assert:

```python
def test_hardware_route_is_registered(self):
    text = read_file("src/WebConfigAsync.h")
    self.assertIn('httpServer.on("/hardware"', text)
    self.assertIn("http_handle_hardware", text)

def test_hardware_page_contains_virtual_button_contract(self):
    text = read_file("src/WebConfigAsync.h")
    self.assertIn("control-press:", text)
    self.assertIn("control-release:", text)
    self.assertIn("addEventListener('hardware'", text)
    self.assertIn("hardwareButton", text)
```

- [ ] **Step 2: Verify contract tests fail**

Run:

```powershell
python -m unittest scripts.test_web_config_contracts
```

Expected: failures because `/hardware` route and page strings do not exist yet.

- [ ] **Step 3: Implement chunked hardware page**

Add `get_hardware_page()`, `get_hardware_page_chunked()`, and `http_handle_hardware()` following the existing page/chunking style. The page should contain six buttons with `data-control="1"` through `data-control="6"` and a six-cell preview grid.

The JavaScript should:

```javascript
const socket = new WebSocket('ws://' + location.hostname + '/ws');
function sendHardwareCommand(command) {
  if (socket.readyState === WebSocket.OPEN) socket.send(command);
}
button.addEventListener('pointerdown', () => sendHardwareCommand('control-press:' + id));
button.addEventListener('pointerup', () => sendHardwareCommand('control-release:' + id));
button.addEventListener('pointercancel', () => sendHardwareCommand('control-release:' + id));
const source = new EventSource('/events');
source.addEventListener('hardware', (event) => updateHardware(JSON.parse(event.data)));
```

- [ ] **Step 4: Register WebSocket commands**

In `onWsEvent()`, handle text messages:

```cpp
int controlNumber = 0;
if (sscanf((const char*)data, "control-press:%d", &controlNumber) == 1) {
  String reason;
  controller_virtual_control_event(constrain(controlNumber, 1, 6) - 1, AceButton::kEventPressed, &reason);
  hardware_send_state();
}
else if (sscanf((const char*)data, "control-release:%d", &controlNumber) == 1) {
  String reason;
  controller_virtual_control_event(constrain(controlNumber, 1, 6) - 1, AceButton::kEventReleased, &reason);
  hardware_send_state();
}
```

- [ ] **Step 5: Register route**

In `http_setup()` add:

```cpp
httpServer.on("/hardware", HTTP_GET, http_handle_hardware);
```

- [ ] **Step 6: Verify tests and build**

Run:

```powershell
python -m unittest scripts.test_web_config_contracts
$env:TMPDIR = "C:\tmp"
$pioExe = "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe"
& $pioExe run -e lilygo-t-display-s3
```

Expected: Python tests pass and S3 build succeeds.

- [ ] **Step 7: Commit**

```powershell
git add src/WebConfigAsync.h scripts/test_web_config_contracts.py
git commit -m "feat: add web hardware test page"
```

### Task 5: Release Notes, Validation, And Size Gate

**Files:**
- Modify: `FORK-RELEASE-NOTES.md`

- [ ] **Step 1: Update fork release notes**

Add a `TBD` commit section describing:

- `/hardware` page
- S3-only WebSocket enablement
- virtual press/release behavior
- live hardware-state events
- compatibility note that saved config format is unchanged
- validation commands and size delta

- [ ] **Step 2: Run full local validation**

Run:

```powershell
python -m unittest scripts.test_validate_config_surfaces scripts.test_web_config_contracts
python scripts/validate_config_surfaces.py
$env:TMPDIR = "C:\tmp"
$pioExe = "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe"
& $pioExe test -e native
& $pioExe run -e lilygo-t-display-s3
& $pioExe run -e lilygo-t-display-s3 -t buildfs
```

Expected: all commands pass.

- [ ] **Step 3: Compare size against baseline**

Report:

- firmware bytes before: `2,408,105`
- firmware bytes after
- app slot bytes: `3,538,944`
- free bytes after
- RAM bytes before: `107,176`
- RAM bytes after

If free app space remains comfortably above 900 KB and runtime page behavior is reasonable, keep WebSocket. If not, branch should switch to HTTP press/release plus polling before review.

- [ ] **Step 4: Commit release notes**

```powershell
git add FORK-RELEASE-NOTES.md
git commit -m "docs: document hardware test page"
```

