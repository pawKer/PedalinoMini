### Incoming MIDI Actions V2 (Grouped Triggers, Multi-Action)

#### Summary
Incoming MIDI Actions now supports **trigger groups** so one incoming MIDI message can run multiple local actions in order.

Locked behavior:
- Scope: bank-scoped configuration using `Global` bank `0` plus banks `1..20`.
- Trigger source types: Control Change and Program Change.
- Actions supported: Set Led Color, Set Slot State, Set Bank.
- Ordering:
  - Within a group: stored action order.
  - Across groups within a bank: group creation order.
- Precedence unchanged: built-in receive behavior first, then incoming trigger groups.
- Bank precedence: `Global` bank `0` runs first, then the active bank.
- Migration policy:
  - Old flat `IncomingActions` is still legacy and must be migrated manually.
  - Older grouped `IncomingTriggers` entries without a trigger-level `Bank` migrate into `Global` bank `0`.

#### Runtime Model
- Deterministic capacities:
  - `INCOMING_TRIGGERS_MAX = 64`
  - `INCOMING_TRIGGER_ACTIONS_MAX = 8`
- Runtime storage:
  - `incomingTrigger *incomingTriggers[BANKS]`
  - `byte incomingTriggerCount[BANKS]`
  - Sparse per-bank allocation; unused banks do not reserve trigger arrays
- Types:
  - `incomingTrigger`:
    - Trigger fields: `triggerType`, `channel`, `number`, `valueMode`, `value`
    - `actionCount`
    - `actions[]` (`incomingTargetAction`)
  - `incomingTargetAction`:
    - `targetAction`, `led`, `color`, `slot`, `state`, `bank`

#### Execution Semantics
- `incoming_actions_run(midiType, channel, data1, data2)`:
  - Match `Global` bank `0` groups first by type/channel/number (+ CC value filter when Exact).
  - Then match the current bank's groups.
  - Execute all actions in each matched group in stored order.
  - Continue scanning and executing later matching groups in the same bank scope.
- Stateful ordering is intentional:
  - If `Set Bank` runs first in a group, later actions in that same group apply to the new bank.
  - If a matched global group changes bank, the same incoming message continues into the newly selected bank's incoming rules.

#### UI (`/incoming-actions`)
- Bank selector uses the same bank model as `/actions`, including `Global`.
- The editor only shows the selected bank's incoming trigger groups.
- `Duplicate Bank To` copies the current bank's incoming rules to another bank.
- Add Trigger button appends a new group.
- Each trigger group includes:
  - Trigger fields
  - Add Action button
  - Delete Group button
- Each action row includes:
  - Action type-specific fields
  - Delete Action button
- No reordering controls (append-only ordering in v2).
- Legacy warning banner appears when old `IncomingActions` is detected in imported JSON.
- Copy behavior replaces the destination bank's incoming rules instead of merging.

#### Persistence + Schema
- JSON config now uses:
  - `IncomingTriggers: [ { Bank, Trigger..., Actions: [ ... ] } ]`
- Legacy:
  - Flat `IncomingActions` is not auto-converted.
- NVS keys updated to grouped storage:
  - Per-bank trigger blobs `InT00` .. `InT20`
  - Per-bank trigger counts `InC00` .. `InC20`
  - Old `InTriggers` / `InTrigCnt` are read as a migration fallback into `Global`

#### Test Plan
1. Single trigger with multiple actions:
   - Configure one CC trigger with 3 actions.
   - Verify all actions run in configured order.
2. Ordering semantics:
   - In one group: `Set Bank` then `Set Slot State` and LED color.
   - Verify latter actions apply after bank switch.
3. Global + bank precedence:
   - Configure matching groups in `Global` and in one numbered bank.
   - Verify `Global` completes before the bank-local group starts.
4. Live bank handoff:
   - Configure a matching global trigger that switches bank, with a matching trigger in the destination bank.
   - Verify the same incoming MIDI event continues into the new bank's rules.
4. Trigger filters:
   - CC Any and Exact behaviors unchanged.
   - PC ignores value filter.
5. CRUD + persistence:
   - Add/delete groups/actions per bank, duplicate bank-to-bank, Apply/Save, reboot, profile switch.
   - Verify order, bank scoping, and payload preservation.
6. Interface coverage:
   - Validate USB, DIN, BLE, RTP, and ipMIDI.
7. Legacy import:
   - Import config with flat `IncomingActions`.
   - Verify no crash and explicit manual migration warning.
8. Grouped trigger migration:
   - Import config with `IncomingTriggers` entries that lack trigger-level `Bank`.
   - Verify they load into `Global` bank `0`.
