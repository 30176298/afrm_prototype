#include "core/validate.hpp"

#include <sstream>

namespace {

// Tests values in range
std::string range_error(const std::string& name, double value, double lo, double hi) {
  if (value >= lo && value <= hi) return "";
  std::ostringstream message;
  message << name << " = " << value << " is outside the allowed range " << lo << " to " << hi;
  return message.str();
}

std::string check_aircraft(const Aircraft& a) {
  if (a.model.empty()) return "the aircraft model name is missing";

  std::string error = range_error("fuel_capacity_kg", a.fuel_capacity_kg, 100, 100000);
  if (error.empty()) error = range_error("takeoff_weight_kg", a.takeoff_weight_kg, 1000, 500000);
  if (error.empty()) error = range_error("fuel_onboard_kg", a.fuel_onboard_kg, 0, 100000);
  if (error.empty()) error = range_error("stores_burn_pct", a.stores_burn_pct, 0, 50);
  if (error.empty()) error = range_error("burn_kg_h_per_t", a.burn_kg_h_per_t, 1, 200);
  if (!error.empty()) return error;

  if (a.fuel_onboard_kg > a.fuel_capacity_kg) return "fuel_onboard_kg is more than fuel_capacity_kg";
  if (a.fuel_onboard_kg >= a.takeoff_weight_kg) return "fuel_onboard_kg must be less than takeoff_weight_kg";
  return "";
}

std::string check_reserves(const Reserves& r) {
  std::string error = range_error("contingency_pct", r.contingency_pct, 0, 100);
  if (error.empty()) error = range_error("alternate_nm", r.alternate_nm, 0, 1000);
  if (error.empty()) error = range_error("final_reserve_min", r.final_reserve_min, 0, 180);
  if (error.empty()) error = range_error("marginal_pct", r.marginal_pct, 0, 100);
  return error;
}

std::string check_flight(const Flight& flight) {
  if (flight.size() < 2) return "the flight data needs at least 2 rows";

  for (std::size_t i = 0; i < flight.size(); ++i) {
    const std::string row = " (row " + std::to_string(i + 1) + ")";

    std::string error = range_error("speed_kt" + row, flight[i].speed_kt, 0, 700);
    if (error.empty()) error = range_error("altitude_ft" + row, flight[i].altitude_ft, 0, 45000);
    if (!error.empty()) return error;

    // "!(a > b)" rather than "a <= b" so that a NaN is rejected too.
    if (i > 0 && !(flight[i].time_s > flight[i - 1].time_s)) return "time_s" + row + " must increase";
    if (i > 0 && !(flight[i].distance_nm >= flight[i - 1].distance_nm)) return "distance_nm" + row + " must not decrease";
  }

  if (!(flight.back().distance_nm > flight.front().distance_nm)) return "the flight must cover a positive distance";
  return "";
}

}  // namespace

std::string validate(const Flight& flight, const Aircraft& aircraft, const Reserves& reserves) {
  std::string error = check_aircraft(aircraft);
  if (error.empty()) error = check_reserves(reserves);
  if (error.empty()) error = check_flight(flight);
  return error;
}
