#pragma once

#include "core/types.hpp"

// Fuel burned per hour in kg/h: burn rate multiplied by the aircraft weight in tonnes, respecting stores
double fuel_flow_kg_h(const Aircraft& aircraft, double weight_kg);

// The same at the aircraft's take-off weight
double fuel_flow_kg_h(const Aircraft& aircraft);

// Fuel burned over the whole flight in kg. The profile is stepped through segment by segment,
// with the flow in each segment set by the current weight, which falls as fuel is burned
double trip_fuel_kg(const Flight& flight, const Aircraft& aircraft);

// whole report - validates the inputs, works out the required fuel, compares it with fuel on board and classifies result
Report assess(const Flight& flight, const Aircraft& aircraft, const Reserves& reserves);

// feasible, marginal, insufficient
const char* status_text(Status status);
