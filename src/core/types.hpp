#pragma once

#include <string>
#include <vector>

// One row of the time-indexed flight profile
struct FlightPoint {
  double time_s;       // seconds since take-off
  double speed_kt;     // airspeed in knots
  double altitude_ft;  // alt in feet
  double distance_nm;  // cumulative distance flown in nautical miles
};

using Flight = std::vector<FlightPoint>;

// Aircraft and fuel state at take-off
struct Aircraft {
  std::string model;
  double fuel_capacity_kg;
  double takeoff_weight_kg;  // includes the fuel on board
  double fuel_onboard_kg;
  double stores_burn_pct;    // extra fuel burn caused by external stores in percent
  double burn_kg_h_per_t;    // fuel burned per hour for each tonne of takeoff weight
};

// Reserve policy chosen by user
struct Reserves {
  double contingency_pct = 5.0;     // of trip fuel
  double alternate_nm = 100.0;      // distance of diversion
  double final_reserve_min = 30.0;  // minutes of flight
  double marginal_pct = 5.0;        // marginal if spare fuel is below this percent of required fuel
};
