### Incoming MIDI Actions V1 (Profile-Scoped, CC/PC Triggers)

#### Summary
Implement a new **Incoming Actions** feature that executes local actions when MIDI-IN messages are received, so external devices can drive Pedalino LED/display/bank state.

Locked behavior:
- Scope: **per profile**, active across **all banks**.
- Sources: all enabled MIDI-IN interfaces (USB, DIN, BLE, RTP, ipMIDI).
- Triggers: **Control Change** and **Program Change** only.
- Matching: type + channel (`1..16` or `Any`) + number (`0..127`), plus optional exact value filter for CC.
- Actions in v1: **Set Led Color**, **Set Slot State**, **Set Bank**.
- Conflict policy: run built-in receive behavior first, then incoming rules (incoming rules win).
- Multi-match: run **all** matching rules in stored list order.
- Ordering UX: creation order only (append on add; no reordering controls).

#### Key Changes
- **Runtime model**
  - Add fixed-capacity rule storage to keep memory deterministic and preserve list order:
    - `INCOMING_ACTIONS_MAX = 64`
    - `incomingAction incomingActions[INCOMING_ACTIONS_MAX]`
    - `byte incomingActionCount`
  - Add `incomingAction` type with:
    - Trigger: `triggerType` (`CC`/`PC`), `channel` (`1..16`, `17=Any`), `number`, `valueMode` (`Any`/`Exact`), `value`
    - Target action: `targetAction` (`Set Led Color`/`Set Slot State`/`Set Bank`)
    - Payload fields:
      - LED: `led` (`1..LEDS`), `color` (`0xRRGGBB`)
      - Slot: `slot` (`1..SLOTS`), `state` (`0/1`)
      - Bank: `bank` (`1..20`)
- **Execution pipeline**
  - Add centralized dispatcher in controller runtime, e.g. `incoming_actions_run(midiType, channel, data1, data2)`.
  - Call dispatcher in MIDI-IN callbacks for CC/PC on all interfaces:
    - `SerialMidiIn.h`, `USBMidiIn.h`, `BLEMidiIn.h`, `UdpMidiIn.h` (RTP + ipMIDI)
  - Call point ordering:
    - Keep existing forward/thru + `leds_update` + existing `switch_profile_or_bank` behavior.
    - Then execute incoming rules.
  - Action application:
    - Set Led Color: set direct LED index, apply `swap_rgb_order`, `FastLED.show()`, update `lastLedColor[currentBank][led]`.
    - Set Slot State: call existing `set_slot_display_state(currentBank, slot-1, state)`.
    - Set Bank: set `currentBank` (`1..20`), then `reset_slot_display_state(currentBank)`, `update_current_step()`, `leds_refresh()`.
- **Web UI**
  - Add top-nav tab and page: `GET /incoming-actions`, `POST /incoming-actions`.
  - Page includes:
    - Add Rule button (append)
    - Rule cards/rows with Delete
    - Trigger fields: message type, channel, number, value mode/value (value enabled only for CC)
    - Action fields:
      - Set Led Color: LED selector + color picker
      - Set Slot State: slot selector + state selector
      - Set Bank: bank selector `1..20`
    - Apply/Save buttons matching existing behavior semantics.
- **Persistence + schema**
  - Persist per profile in both SPIFFS JSON and NVS.
  - JSON: add top-level `IncomingActions` array in profile config.
  - NVS: add profile keys for incoming rules and count.
  - Extend `data/schema.json` with `IncomingActions` definition.
  - Backward compatibility:
    - Missing `IncomingActions` => `incomingActionCount = 0`.
    - Legacy configs load unchanged.
  - Include `IncomingActions` in import/export and profile save/load flows.

#### Public Interfaces / Types
- New HTTP endpoints:
  - `GET /incoming-actions`
  - `POST /incoming-actions`
- New profile config section:
  - `IncomingActions: []`
- New runtime type/constants:
  - `incomingAction` struct
  - `INCOMING_ACTIONS_MAX`
  - `incomingActions[]`, `incomingActionCount`

#### Test Plan
1. **UI CRUD + ordering**
   - Add multiple rules, verify append order, delete middle rule, Apply/Save behavior.
2. **Persistence**
   - Save rules, reboot, verify rules restored; switch profiles A/B/C and verify profile isolation.
3. **Trigger matching**
   - CC rule with `Any` value fires on all values.
   - CC rule with exact value fires only on matching value.
   - PC rule ignores value filter and matches channel+program number.
4. **Action outcomes**
   - Set Led Color updates correct physical LED and persists visual state.
   - Set Slot State updates S3 slot state rendering.
   - Set Bank switches to selected bank `1..20` and refreshes LEDs/display state.
5. **Source interfaces**
   - Validate same rule behavior from USB, DIN, BLE, RTP, and ipMIDI inputs (when enabled).
6. **Precedence/conflicts**
   - Message that also triggers existing `leds_update` path: confirm incoming action result is final visible state.
7. **Regression**
   - No changes to outgoing action behavior, actions tab behavior, display customization tab behavior, or non-triggered MIDI traffic.

#### Assumptions
- Incoming rules are profile-level only (not per-bank, not global across profiles).
- `Set Bank` is fixed target per rule (not derived from incoming value).
- Incoming channel wildcard uses `17` internally (`Any`) to align with existing “All” channel semantics.
- No rule enable/disable toggle in v1; delete to disable.
