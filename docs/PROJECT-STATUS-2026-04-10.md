# PedalinoMini Project Status (2026-04-10)

## Snapshot
- Primary target in focus: `lilygo-t-display-s3` (ESP32-S3).
- Latest implemented feature: **Sequence Names in Actions/Sequences UI**.
- Build status: `pio run -e lilygo-t-display-s3` passes.

## Implemented Features (Current)
1. **S3 Display Improvement V2**
- Fixed 2x3 grid + top bar.
- Runtime slot state model (`off/on`) with `Set Slot State` display-only action.
- Temporary overlay behavior restored (~1.5s), with flicker fixes.
- Tag semantics for ON/OFF preserved (`tag1`=ON, `tag0`=OFF).

2. **Display Customization Tab**
- Dedicated `/display` page with 6 slot border color pickers and mock preview.
- Border colors are profile-scoped via `slotBorderColor[SLOTS]`.
- Persisted in profile JSON/NVS using `DisplaySlots`.
- Legacy config compatibility preserved.

3. **Sequence Names (Newly Implemented)**
- Added profile-scoped sequence names (`max 16 chars`).
- New runtime storage: `sequenceNames[SEQUENCES][MAXSEQUENCENAME+1]`.
- Added schema/config field: top-level `SequenceNames`.
- Added NVS profile key: `SeqNames`.
- Sequences page now supports editing `sequencename`.
- Sequence selectors now render labels as:
  - `N` when name is empty
  - `N (Name)` when set
- Applied in:
  - Actions tab sequence dropdowns
  - Sequences tab nested sequence dropdowns
  - Sequences tab selector buttons and title

## Validation Status
- `data/schema.json` parses correctly.
- Firmware build validated successfully for:
  - `lilygo-t-display-s3`
- No new compile errors introduced by sequence-name changes.

## Current Workspace State
- Feature docs have been reorganized to numbered folders under `docs/features/`.
- Old feature doc paths are currently shown as deleted and new paths as untracked in git status (rename/move not committed yet).
- Code changes currently staged only in working tree (not committed in this snapshot):
  - `src/Pedalino.h`
  - `src/Config.h`
  - `src/WebConfigAsync.h`
  - `data/schema.json`

## Planned / Next Feature (Not Yet Implemented)
- **Incoming MIDI Actions V1** (`docs/features/5-midi-in-actions/midi-in-actions.md`)
  - Profile-scoped incoming-rule engine for CC/PC.
  - Rule-triggered local actions (`Set Led Color`, `Set Slot State`, `Set Bank`).
  - New web UI page and persistence/schema additions.

## Recommended Next Steps
1. Hardware validation pass for sequence names across profile switch/reboot/export-import.
2. Commit the docs folder move/rename cleanly (to avoid mixed delete/add status).
3. Start implementation of Incoming MIDI Actions V1.
