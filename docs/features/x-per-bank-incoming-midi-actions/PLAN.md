# Bank-Scoped Incoming MIDI Actions With Duplicate-Bank Workflow

## Summary
Move incoming MIDI actions from one profile-wide ruleset to the same bank model used by normal actions: a `Global` bank 0 ruleset plus optional bank-specific rules for banks 1..20. The `/incoming-actions` page should edit one selected bank at a time, and expose a `Duplicate Bank To` dropdown that copies the current bank’s incoming rules into another bank by replacing the destination bank’s incoming rules.

## Key Changes
- Runtime model:
  - Replace the single profile-scoped `incomingTriggers[]` + `incomingTriggerCount` store with bank-scoped storage covering `0..BANKS-1`.
  - Keep per-trigger action ordering and per-bank trigger ordering unchanged.
  - Dispatch incoming MIDI by scanning `Global` bank 0 rules first, then the active bank’s rules.
  - Match the user-selected live behavior: if a matched `Global` rule changes `currentBank`, continue the same receive event using the new current bank’s incoming rules.
- Persistence and schema:
  - Extend JSON `IncomingTriggers` entries with a `Bank` field rather than inventing a second top-level structure.
  - Treat missing `Bank` on import as `0` so existing profile-scoped configs migrate into `Global` automatically.
  - Update `data/schema.json` so `IncomingTriggers[*].Bank` accepts `0..20`.
  - Replace the single NVS blob/count (`InTriggers` / `InTrigCnt`) with per-bank incoming-trigger storage, mirroring the existing per-bank action persistence style so sparse configs do not reserve a full 21-bank fixed array in RAM/NVS.
- Web UI:
  - Add the same bank selector pattern used on `/actions`, including `Global` bank 0.
  - Filter the `/incoming-actions` editor to the selected bank only.
  - Add a `Duplicate Bank To` dropdown on `/incoming-actions`, following the current actions-page interaction model.
  - Copy source = currently selected bank; destination = selected dropdown target.
  - Copy semantics: replace the destination bank’s incoming rules entirely, then rebuild the page in that destination context or keep current context with a success message.
  - Keep add/delete/apply/save behavior scoped to the currently selected bank.
- Code surfaces to update:
  - `src/Pedalino.h`: bank-scoped incoming-trigger storage declarations and related helpers.
  - `src/Controller.h`: bank-aware incoming dispatch logic and precedence/order behavior.
  - `src/Config.h`: JSON import/export plus NVS read/write migration logic.
  - `src/WebConfigAsync.h`: bank selector, filtered rendering, duplicate-bank command, and bank-scoped POST handling.
  - `data/schema.json`: `IncomingTriggers[*].Bank` validation.

## Test Plan
- Migration:
  - Load an existing config whose `IncomingTriggers` entries have no `Bank`.
  - Verify all imported rules land in `Global` bank 0 and behavior matches current profile-wide behavior.
- Dispatch:
  - Verify `Global` rules run before current-bank rules.
  - Verify a message matching only bank-local rules does not trigger unrelated banks.
  - Verify a `Global` rule with `Set Bank` causes the same receive event to continue into the newly selected bank’s rules.
  - Verify intra-group ordering still works when `Set Bank` is followed by LED or slot-state actions.
- UI and copy:
  - Edit rules in `Global`, bank 1, and bank 2 independently.
  - Duplicate bank 1 to bank 2 and confirm bank 2 is replaced, not merged.
  - Duplicate `Global` to a numbered bank and a numbered bank to `Global`.
- Persistence:
  - Save, reboot, switch profiles, and confirm per-bank incoming rules reload correctly.
  - Confirm Apply affects runtime immediately and Save persists the selected bank’s incoming rules.
- Regression:
  - Confirm existing supported incoming action types remain `Set Led Color`, `Set Slot State`, and `Set Bank`.
  - Confirm legacy `IncomingActions` warning behavior stays intact.

## Assumptions
- “Like actions work” means `Global` bank 0 plus bank-specific rules, not bank-only replacement.
- The `/incoming-actions` page should reuse the actions-page bank-selection UX instead of introducing a separate source/target form.
- Duplicate UX should match the existing `Duplicate Bank To` dropdown pattern, but incoming-rule copy will use replace semantics even though the current actions copy path effectively merges.
- Old profile-scoped incoming-trigger configs should migrate forward automatically to `Global` bank 0 rather than requiring manual recreation.
