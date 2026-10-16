#pragma once

#include "core/types.hpp"

//small worked example
inline Flight example_flight() {
  return {{0, 240, 0, 0}, {600, 240, 10000, 40}, {1200, 240, 10000, 80}, {1800, 240, 0, 120}};
}

//example aircraft
inline Aircraft example_aircraft() { return {"EXAMPLE", 2000, 10000, 700, 0, 50}; }
