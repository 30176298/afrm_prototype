#include "core/fuel.hpp"
#include "core/validate.hpp"
#include <algorithm>

double fuel_flow_kg_h(const Aircraft& aircraft) {
  const double tonnes = aircraft.takeoff_weight_kg / 1000.0;
  return aircraft.burn_kg_h_per_t * tonnes * (1.0 + aircraft.stores_burn_pct / 100.0);
}

double trip_fuel_kg(const Flight& flight, const Aircraft& aircraft) {
  const double hours = (flight.back().time_s - flight.front().time_s) / 3600.0;
  return fuel_flow_kg_h(aircraft) * hours;
}

Report assess(const Flight& flight, const Aircraft& aircraft, const Reserves& reserves) {
  Report report;
  report.error = validate(flight, aircraft, reserves);
  if (!report.error.empty()) return report;  // invalid input: calculate nothing

  const double distance_nm = flight.back().distance_nm - flight.front().distance_nm;
  report.trip_kg = trip_fuel_kg(flight, aircraft);
  const double burn_per_nm = report.trip_kg / distance_nm;

  // Reserves
  report.contingency_kg = report.trip_kg * reserves.contingency_pct / 100.0;
  report.alternate_kg = burn_per_nm * reserves.alternate_nm;
  report.final_reserve_kg = fuel_flow_kg_h(aircraft) * reserves.final_reserve_min / 60.0;

  // Compare with the fuel actually on board
  report.required_kg =
      report.trip_kg + report.contingency_kg + report.alternate_kg + report.final_reserve_kg;
  report.margin_kg = aircraft.fuel_onboard_kg - report.required_kg;

  if (report.margin_kg < 0.0) {
    report.status = Status::Insufficient;
  } else if (report.margin_kg < report.required_kg * reserves.marginal_pct / 100.0) {
    report.status = Status::Marginal;
  } else {
    report.status = Status::Feasible;
  }

  // Maximum range
  const double range_nm = (aircraft.fuel_onboard_kg - report.final_reserve_kg) / burn_per_nm - reserves.alternate_nm;
  report.max_range_nm = std::max(0.0, range_nm / (1.0 + reserves.contingency_pct / 100.0));
  return report;
}

const char* status_text(Status status) {
  switch (status) {
    case Status::Feasible: return "Feasible";
    case Status::Marginal: return "Marginal";
    default: return "Insufficient";
  }
}
