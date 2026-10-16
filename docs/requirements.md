# AFRM requirements


## Functional

| ID | The system shall... | Verified by |
|----|---------------------|-------------|
| FR-01 | load a time-indexed flight profile (time, speed, altitude, cumulative distance) from a CSV file | A |
| FR-02 | load aircraft data (model, fuel capacity, take-off weight, fuel on board, stores drag, performance parameters) from a CSV file | A |
| FR-03 | calculate trip fuel from a fixed fuel flow, set by the aircraft's take-off weight, and the flight time | A |
| FR-04 | increase the fuel flow by a set percentage when external stores are carried | A |
| FR-05 | calculate contingency, alternate and final reserve fuel from user-set constraints, and add them to trip fuel to give the required fuel | A |
| FR-06 | classify the flight as Feasible, Marginal or Insufficient by comparing the fuel on board with the required fuel | A |
| FR-07 | calculate the maximum range achievable with the fuel on board, using the same profile and reserves | A |
| FR-08 | present the inputs, results and status in a graphical interface | M |

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

