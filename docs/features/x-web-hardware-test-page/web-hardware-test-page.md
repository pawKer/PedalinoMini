# Web Hardware Test Page

## Summary
Add a new Web UI page at `/hardware` that acts as a browser-side version of the 6 main momentary footswitches for testing.  
V1 will provide:
- 6 virtual buttons mapped to fixed `Controls 1-6`
- press/release behavior that uses the same runtime action pipeline as the physical switches
- a live, read-only 2x3 preview of the current hardware display state
- live tag/slot updates when state changes from the browser, real footswitches, or incoming MIDI

## Key Changes
### Web UI
- Add a dedicated `/hardware` page instead of extending `/display`.
- Show 6 large virtual footswitch buttons in the same left-to-right order as the physical controller.
- Each button targets the matching configured control index (`Control 1` through `Control 6`), not the current slot index.
- Button labels should be bank-aware:
  - use the current runtime tag/label for that control when available
  - fall back to `Control N` when no active label is available
- Include a read-only 2x3 grid preview under or beside the buttons that mirrors the current display slots, including current text and slot state.

### Runtime integration
- Reuse the existing button-event pipeline in [`src/Controller.h`](/c:/Users/Rares/Desktop/Coding/PedalinoMini/src/Controller.h) by adding a small helper that injects synthetic `Pressed` and `Released` events for a selected control’s primary pedal/button mapping.
- Do not add a second “run action” path; virtual presses must behave like hardware presses so existing action logic, tags, bank behavior, and incoming-MIDI side effects stay consistent.
- Support only controls that resolve to a single momentary-style pedal/button input in v1.
- If a control is unmapped or not compatible with virtual pressing, render its button disabled and show a short reason in the page.

### Browser transport and live state
- Extend the existing WebSocket handler in [`src/WebConfigAsync.h`](/c:/Users/Rares/Desktop/Coding/PedalinoMini/src/WebConfigAsync.h) with two string commands:
  - `control-press:N`
  - `control-release:N`
- Use pointer/touch down to send `control-press:N` and pointer/touch up or cancel to send `control-release:N`.
- Add a compact runtime state payload on the existing `/events` stream that includes:
  - current bank id/name
  - current button labels for Controls 1-6
  - current 2x3 slot labels
  - current slot state for each visible slot
- Add a single helper in [`src/DisplayTFT.h`](/c:/Users/Rares/Desktop/Coding/PedalinoMini/src/DisplayTFT.h) or shared runtime code to publish that state whenever visible UI state changes.
- Emit updates after:
  - action processing completes
  - current bank changes
  - incoming MIDI actions update tags or slot state
  - any other runtime path that changes the visible 2x3 display state

## Public Interfaces
- New route: `GET /hardware`
- New WebSocket commands:
  - `control-press:N`
  - `control-release:N`
- New SSE event type:
  - `hardware` with a compact JSON payload for the 6 button labels and 2x3 preview state

## Test Plan
1. Open `/hardware` and verify the page loads with 6 buttons plus a 2x3 preview matching the current bank.
2. Press and release each virtual button and confirm actions configured for `Press`, `Release`, and `Press & Release` behave exactly like the real switch.
3. Hold a virtual button and confirm only the `Press` event fires until release.
4. Change bank from elsewhere in the UI/device and confirm button labels and the 2x3 preview update without reloading.
5. Trigger incoming MIDI that changes slot state or tags and confirm the page updates live.
6. Verify unmapped or unsupported controls show as disabled instead of failing silently.
7. Verify the page does not alter saved configuration; it is runtime-only.
8. Verify `/display` remains a display-settings page and is unaffected.

## Assumptions and Defaults
- V1 is a testing surface, not a configuration editor.
- The page targets fixed `Controls 1-6`, because those correspond most closely to the physical footswitches.
- The 2x3 grid is read-only in v1; only the dedicated virtual buttons are clickable.
- V1 supports standard momentary-style testing only; simultaneous multi-button gestures and non-footswitch control types are out of scope for the first version.
- `/virtualpedals` remains the existing controls editor; this feature should not repurpose that route.
