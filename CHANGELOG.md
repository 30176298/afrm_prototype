# Changelog

Changes to this pplication are here recorded, the numbering scheme is summarised as (MAJOR.MINOR.PATCH)
Bugfixes, test & config changes raises the PATCH number
Changes or additions to behaviour raises the MINOR number
Changes to data formats or public interface functions raises the MAJOR number

After initial baseline v1.0.0, changes are implemented via change process in README.md,
github issues, pull requests

## [1.1.0] - 2026-10-30

CR-001: weight-dependent fuel-burn model. Results are less conservative than v1.0.x, because a lighter aircraft burns less fuel, offset by new taxi allowance. Flights near a threshold can change status

### Changes
- Trip fuel now steps through the flight profile, with the fuel flow in each segment set by the current weigh (FR-03)
- Alternate and final reserve are now based on the estimated landing weight (FR-05)
- Added taxi fuel allowance, default 100 kg, validated 0-1000 kg, shown in the GUI (FR-09)
- Maximum range uses the same reserves, so its values change (FR-07 text unchanged)
- Requirements revised
- Added tests/oracle/fuel_oracle.py
- 12 new tests 12 existing tests updated

## [1.0.1] - 2026-10-28

### Changes
- Added coverage preset fo CMake
- Added coverage job to CI
- Updated CI to cover all branches
- Added GitHub rules for pull request and CI pass prior to main merge

## [1.0.0] - 2026-10-28

Controlled baseline to define boundary between assignments 1 and 2. No significant changes made, just the addition of the change process docs.

### Changes
- Project version raised from 0.1.0 to 1.0.0
- Added this changelog and a configuration management section to README.md

## [0.1.0] - 2026-10 (Assessment 1)

### Chanegs
- Repository template, environment configuration
- Architecture diagram, requirements.md
- Simulated flight and aircraft data read
- Input validation and associated tests
- Simple fuel burn model
- GUI, CI jobs, bad data tests
