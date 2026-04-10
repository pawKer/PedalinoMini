### Incoming MIDI Actions V2 (Grouped Triggers, Multi-Action)

#### Summary
Incoming MIDI Actions now supports **trigger groups** so one incoming MIDI message can run multiple local actions in order.

Locked behavior:
- Scope: profile-level configuration.
- Trigger source types: Control Change and Program Change.
- Actions supported: Set Led Color, Set Slot State, Set Bank.
- Ordering:
  - Within a group: stored action order.
  - Across groups: group creation order.
- Precedence unchanged: built-in receive behavior first, then incoming trigger groups.
- Migration policy: old flat `IncomingActions` is legacy and must be migrated manually.

#### Runtime Model
- Deterministic capacities:
  - `INCOMING_TRIGGERS_MAX = 64`
  - `INCOMING_TRIGGER_ACTIONS_MAX = 4`
- Runtime storage:
  - `incomingTrigger incomingTriggers[INCOMING_TRIGGERS_MAX]`
  - `byte incomingTriggerCount`
- Types:
  - `incomingTrigger`:
    - Trigger fields: `triggerType`, `channel`, `number`, `valueMode`, `value`
    - `actionCount`
    - `actions[]` (`incomingTargetAction`)
  - `incomingTargetAction`:
    - `targetAction`, `led`, `color`, `slot`, `state`, `bank`

#### Execution Semantics
- `incoming_actions_run(midiType, channel, data1, data2)`:
  - Match each trigger group by type/channel/number (+ CC value filter when Exact).
  - Execute all actions in the matched group in stored order.
  - Continue scanning and executing later matching groups.
- Stateful ordering is intentional:
  - If `Set Bank` runs first in a group, later actions in that same group apply to the new bank.

#### UI (`/incoming-actions`)
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

#### Persistence + Schema
- JSON config now uses:
  - `IncomingTriggers: [ { Trigger..., Actions: [ ... ] } ]`
- Legacy:
  - Flat `IncomingActions` is not auto-converted.
- NVS keys updated to grouped storage:
  - `InTriggers`
  - `InTrigCnt`

#### Test Plan
1. Single trigger with multiple actions:
   - Configure one CC trigger with 3 actions.
   - Verify all actions run in configured order.
2. Ordering semantics:
   - In one group: `Set Bank` then `Set Slot State` and LED color.
   - Verify latter actions apply after bank switch.
3. Multi-group matching:
   - Two groups matching the same incoming CC.
   - Verify group A completes before group B starts.
4. Trigger filters:
   - CC Any and Exact behaviors unchanged.
   - PC ignores value filter.
5. CRUD + persistence:
   - Add/delete groups/actions, Apply/Save, reboot, profile switch.
   - Verify order and payload preservation.
6. Interface coverage:
   - Validate USB, DIN, BLE, RTP, and ipMIDI.
7. Legacy import:
   - Import config with flat `IncomingActions`.
   - Verify no crash and explicit manual migration warning.
