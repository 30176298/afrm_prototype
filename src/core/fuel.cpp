#include "core/fuel.hpp"

double fuel_flow_kg_h(const Aircraft& aircraft) {
  const double tonnes = aircraft.takeoff_weight_kg / 1000.0;
  return aircraft.burn_kg_h_per_t * tonnes * (1.0 + aircraft.stores_burn_pct / 100.0);
}

double trip_fuel_kg(const Flight& flight, const Aircraft& aircraft) {
  const double hours = (flight.back().time_s - flight.front().time_s) / 3600.0;
  return fuel_flow_kg_h(aircraft) * hours;
}
