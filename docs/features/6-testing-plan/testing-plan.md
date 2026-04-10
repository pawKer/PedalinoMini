## Live-Safety Hardening Review Plan (S3-Only, Backward-Compatible)

### Summary
Goal: produce and execute a **balanced, decision-complete reliability hardening plan** so PedalinoMini is safer for live use on LilyGO T-Display S3, while keeping config compatibility.

Initial high-risk findings from exploration:
1. Cross-core shared-state mutation risk (Web handlers mutate runtime state while controller/MIDI loops run).
2. Heavy dynamic allocation and linked-list mutation in runtime/config paths.
3. Large duplicated logic in MIDI-in and Web action handling.
4. No formal automated test harness (`test/` missing).

### Key Changes
1. **P0: Runtime Safety and State Ownership**
- Adopt a **single-writer config-apply model**:
  - Web/API layer stops mutating live structures directly.
  - Web posts build a staged `PendingProfileDelta`.
  - Controller loop applies delta at a safe sync point, then atomically flips a generation counter.
- Introduce one shared `stateLock` policy:
  - Writes only in apply path.
  - Reads in hot paths remain lock-free where safe, otherwise bounded lock scope.
- Replace crash-prone `assert` behavior in live paths with recoverable guards + error counters/alerts.
- Add bounded-failure behavior for invalid action/rule payloads (drop rule/action, keep controller alive).

2. **P0/P1: Data Model Hardening (Backward Compatible)**
- Keep persisted schema backward compatible; add explicit `ConfigVersion` and migration hooks.
- Internally migrate from bank linked-lists to **indexed contiguous action storage**:
  - `actionsByBank[bank][index]` + `actionCount[bank]` for deterministic iteration and safer edits.
  - Keep existing JSON shape on export/import during transition.
- Add strict load validation:
  - clamp + reject-invalid strategy with per-field diagnostics.
  - preserve bootability even on partially corrupt configs.
- Add schema consistency checks between runtime enums and JSON schema at build-time script step.

3. **P1: UI and Duplication Cleanup**
- Redesign action editor UX to reduce operator error:
  - consistent trigger/action templates,
  - action-specific fields only,
  - inline validation and conflict warnings before Apply/Save.
- Extract shared parsers/formatters (color, channel, slots, ranges) into reusable helpers.
- Unify MIDI IN callback logic into one dispatcher helper (interface-agnostic), minimizing copy-paste divergence.

4. **P1: Quality and Release Gates**
- Introduce static checks and warning budgets:
  - compiler warnings tracked and triaged,
  - static analysis integrated into CI (when lock/permission issues resolved).
- Add “live release gate” checklist and automated preflight command set for S3 firmware builds.

### Public Interfaces / Types
- New internal runtime types:
  - `PendingProfileDelta`
  - `RuntimeStateGeneration`
  - indexed action-bank containers (`actionCount[]` + contiguous storage)
- Public config additions (backward compatible):
  - top-level `ConfigVersion`
  - optional migration metadata block for diagnostics
- Web/API behavior change:
  - `Apply/Save` become staged + synchronized commit operations (no direct live mutation).

### Test Plan
1. **Automated (Host/Static)**
- Parser/validation unit tests for config import/export edge cases.
- Migration tests: legacy config -> current runtime -> export -> reimport round-trip.
- Rule/action bounds tests (invalid channel/code/value/slot/bank).
- Enum↔schema consistency tests.

2. **Firmware Integration (S3)**
- Build and boot smoke tests for normal + degraded configs.
- Concurrency tests:
  - rapid Web Apply/Save while MIDI input storm is active,
  - repeated profile/bank switching under load.
- Runtime stability:
  - no crashes/assert resets,
  - deterministic state after repeated apply cycles.

3. **Hardware Lab Validation (Strong)**
- 2-hour soak with mixed USB/DIN/BLE/RTP/ipMIDI traffic.
- Stress scenarios:
  - rapid footswitch spam,
  - incoming MIDI rule bursts,
  - repeated Wi-Fi reconnect + web edits.
- Recovery scenarios:
  - malformed import attempts,
  - mid-session profile swaps,
  - power-cycle persistence checks.

4. **Live-Readiness Acceptance Criteria**
- Zero crash/reboot in soak run.
- All safety-critical edits applied without state corruption.
- Backward compatibility verified on representative legacy configs.
- S3 build + validation checklist fully green.

### Assumptions and Defaults
- Scope is **S3 only now**; other boards remain functional but not first-priority optimized.
- Backward compatibility is mandatory for user configs.
- Strong lab validation is required before declaring “live safe.”
- Improvements are phased: P0 safety first, then model/UI cleanup.
