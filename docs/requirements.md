# AFRM requirements

## Revision history

| Rev | Version | Change record | Summary |
|-----|---------|---------------|---------|
| 1 | v1.0.0 | Assessment 1 baseline | Original 18 requirements |
| 2 | v1.1.0 | CR-001 | FR-03 and FR-05 revised, FR-09 added, `taxi_kg` limit added to DR-03. FR-07 text is unchanged, but test data will change |

## Functional

| ID | The system shall... | Verified by |
|----|---------------------|-------------|
| FR-01 | load a time-indexed flight profile (time, speed, altitude, cumulative distance) from a CSV file | A |
| FR-02 | load aircraft data (model, fuel capacity, take-off weight, fuel on board, stores drag, performance parameters) from a CSV file | A |
| FR-03 | calculate trip fuel by stepping through the flight profile, with the fuel flow in each segment set by the aircraft's current weight, which falls as fuel is burned | A |
| FR-04 | increase the fuel flow by a set percentage when external stores are carried | A |
| FR-05 | calculate contingency, alternate and final reserve fuel from user-set constraints, with the alternate and final reserve based on the estimated landing weight, and add them and the taxi allowance to trip fuel to give the required fuel | A |
| FR-06 | classify the flight as Feasible, Marginal or Insufficient by comparing the fuel on board with the required fuel | A |
| FR-07 | calculate the maximum range achievable with the fuel on board, using the same profile and reserves | A |
| FR-08 | present the inputs, results and status in a graphical interface | M |
| FR-09 | include a user-set taxi-fuel allowance in the required fuel, and report it separately | A |

## Data validation

| ID | The system shall... | Verified by |
|----|---------------------|-------------|
| DR-01 | reject a data file whose column headings do not state the expected units | A |
| DR-02 | reject a file that is empty, malformed, or contains non-numeric or non-finite values, and say which line is at fault | A |
| DR-03 | reject values outside defined physical limits, naming the value and the limits | A |
| DR-04 | reject a flight profile with fewer than two rows, whose time does not increase, whose distance decreases, or that covers no distance | A |
| DR-05 | reject aircraft data with no model name, or with fuel on board above the fuel capacity or the take-off weight | A |
| DR-06 | perform no calculation when validation fails, and report the reason | A |

## Non-functional

| ID | The system shall... | Verified by |
|----|---------------------|-------------|
| NFR-01 | keep the calculation core free of any GUI dependency, so it builds and is tested without one | CI |
| NFR-02 | assess a 10,000-row flight profile in under 100 ms | A |
| NFR-03 | compile without warnings under strict settings on both GCC and MSVC | CI |
| NFR-04 | avoid redrawing continuously while idle | M |

