#include "core/fuel.hpp"
#include "core/validate.hpp"
#include <algorithm>
#include <cstddef>

double fuel_flow_kg_h(const Aircraft& aircraft, double weight_kg) {
  const double tonnes = weight_kg / 1000.0;
  return aircraft.burn_kg_h_per_t * tonnes * (1.0 + aircraft.stores_burn_pct / 100.0);
}

double fuel_flow_kg_h(const Aircraft& aircraft) {
  return fuel_flow_kg_h(aircraft, aircraft.takeoff_weight_kg);
}

double trip_fuel_kg(const Flight& flight, const Aircraft& aircraft) {
  double weight_kg = aircraft.takeoff_weight_kg;
  double trip_kg = 0.0;
  for (std::size_t i = 1; i < flight.size(); ++i) {
    const double hours = (flight[i].time_s - flight[i - 1].time_s) / 3600.0;
    // The burn cannot exceed the weight left
    const double burn_kg = std::min(fuel_flow_kg_h(aircraft, weight_kg) * hours, weight_kg);
    trip_kg += burn_kg;
    weight_kg -= burn_kg;
  }
  return trip_kg;
}

Report assess(const Flight& flight, const Aircraft& aircraft, const Reserves& reserves) {
  Report report;
  report.error = validate(flight, aircraft, reserves);
  if (!report.error.empty()) return report;  // invalid input: calculate nothing

  const double distance_nm = flight.back().distance_nm - flight.front().distance_nm;
  const double hours = (flight.back().time_s - flight.front().time_s) / 3600.0;
  report.trip_kg = trip_fuel_kg(flight, aircraft);
  const double burn_per_nm = report.trip_kg / distance_nm;

  // Alternate and final reserve flown at weight on landing, with reserves still on board
  // Alternate flown at average ground speed of the trip
  const double landing_weight_kg = aircraft.takeoff_weight_kg - report.trip_kg;
  const double landing_flow_kg_h = fuel_flow_kg_h(aircraft, landing_weight_kg);

  // Reserves
  report.contingency_kg = report.trip_kg * reserves.contingency_pct / 100.0;
  report.alternate_kg = landing_flow_kg_h * hours * reserves.alternate_nm / distance_nm;
  report.final_reserve_kg = landing_flow_kg_h * reserves.final_reserve_min / 60.0;
  report.taxi_kg = reserves.taxi_kg;

  // Compare with the fuel actually on board
  report.required_kg = report.trip_kg + report.contingency_kg + report.alternate_kg +
                       report.final_reserve_kg + report.taxi_kg;
  report.margin_kg = aircraft.fuel_onboard_kg - report.required_kg;

  if (report.margin_kg < 0.0) {
    report.status = Status::Insufficient;
  } else if (report.margin_kg < report.required_kg * reserves.marginal_pct / 100.0) {
    report.status = Status::Marginal;
  } else {
    report.status = Status::Feasible;
  }

  // Maximum range:
  const double trip_allowance_kg = (aircraft.fuel_onboard_kg - report.taxi_kg - report.final_reserve_kg - report.alternate_kg) /
                                   (1.0 + reserves.contingency_pct / 100.0);
  report.max_range_nm = std::max(0.0, trip_allowance_kg / burn_per_nm);
  return report;
}

const char* status_text(Status status) {
  switch (status) {
    case Status::Feasible: return "Feasible";
    case Status::Marginal: return "Marginal";
    default: return "Insufficient";
  }
}
