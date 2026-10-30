#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>

#include "core/fuel.hpp"
#include "helpers.hpp"

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

TEST_CASE("FR-03 fuel flow matches the hand calculation", "[FR-03]") {
  // 50 kg/h per tonne x 10 tonnes = 500 kg/h
  REQUIRE_THAT(fuel_flow_kg_h(example_aircraft()), WithinAbs(500.0, 1e-9));
}

TEST_CASE("FR-03 fuel flow follows the weight it is given", "[FR-03]") {
  // 50 kg/h per tonne x 9 tonnes = 450 kg/h
  REQUIRE_THAT(fuel_flow_kg_h(example_aircraft(), 9000.0), WithinAbs(450.0, 1e-9));
  REQUIRE_THAT(fuel_flow_kg_h(example_aircraft(), 10000.0), WithinAbs(fuel_flow_kg_h(example_aircraft()), 1e-9));
}

TEST_CASE("FR-03 trip fuel matches the hand calculation", "[FR-03]") {
  // Three 600 s segments (1/6 h), 50 kg/h per tonne, starting at 10,000 kg:
  //   10,000 kg -> 500 kg/h        -> 83.3333 kg
  //    9,916.667 kg -> 495.833 kg/h -> 82.6389 kg
  //    9,834.028 kg -> 491.701 kg/h -> 81.9502 kg      total 247.9225 kg
  REQUIRE_THAT(trip_fuel_kg(example_flight(), example_aircraft()), WithinAbs(247.9225, 1e-4));
}

TEST_CASE("FR-03 fuel burn falls as the aircraft gets lighter", "[FR-03]") {
  // First half hour: 500 kg/h x 0.5 h = 250 kg. The aircraft is then 9,750 kg, so the
  // second half hour burns 487.5 kg/h x 0.5 h = 243.75 kg.
  const Flight flight = {{0, 240, 0, 0}, {1800, 240, 0, 60}, {3600, 240, 0, 120}};
  REQUIRE_THAT(trip_fuel_kg(flight, example_aircraft()), WithinAbs(493.75, 1e-9));
}

TEST_CASE("FR-03 the weight-dependent trip fuel is below the constant-flow value", "[FR-03]") {
  const double constant_flow_kg = fuel_flow_kg_h(example_aircraft()) * 0.5;  // 250 kg
  const double trip_kg = trip_fuel_kg(example_flight(), example_aircraft());
  REQUIRE(trip_kg < constant_flow_kg);
  REQUIRE(trip_kg > 0.98 * constant_flow_kg);
}

TEST_CASE("FR-03 a longer flight burns proportionally more", "[FR-03]") {
  const Flight short_flight = {{0, 240, 0, 0}, {900, 240, 0, 60}};  // 15 minutes
  const Flight long_flight = {{0, 240, 0, 0}, {2700, 240, 0, 180}}; // 45 minutes
  REQUIRE_THAT(trip_fuel_kg(short_flight, example_aircraft()), WithinAbs(125.0, 1e-9));
  REQUIRE_THAT(trip_fuel_kg(long_flight, example_aircraft()), WithinAbs(375.0, 1e-9));
}

TEST_CASE("FR-03 a heavier aircraft burns more fuel", "[FR-03]") {
  Aircraft heavy = example_aircraft();
  heavy.takeoff_weight_kg = 11000;
  // 550 kg/h for 600 s, then the weight falls (11,000 -> 10,908.33 -> 10,817.36 kg)
  REQUIRE_THAT(trip_fuel_kg(example_flight(), heavy), WithinAbs(272.7147, 1e-4));
}

TEST_CASE("FR-03 stepwise burn agrees with the closed-form solution", "[FR-03]") {
  const Aircraft aircraft = {"BIG", 9000, 38000, 4000, 0, 36};
  Flight flight;
  for (int i = 0; i <= 60; ++i) flight.push_back({60.0 * i, 240, 0, 4.0 * i});
  const double c = 36.0 / 1000.0 / 3600.0;  // per second
  const double exact_kg = 38000.0 * (1.0 - std::exp(-c * 3600.0));
  REQUIRE_THAT(trip_fuel_kg(flight, aircraft), WithinRel(exact_kg, 0.001));
}

TEST_CASE("FR-03 burn can never exceed the aircraft weight", "[FR-03]") {
  Aircraft aircraft = example_aircraft();
  aircraft.burn_kg_h_per_t = 200;  // upper validation limits
  aircraft.stores_burn_pct = 50;
  const Flight flight = {{0, 240, 0, 0}, {1000000, 240, 0, 80}};  // far longer than any fuel load
  const double trip_kg = trip_fuel_kg(flight, aircraft);
  REQUIRE(std::isfinite(trip_kg));
  REQUIRE(trip_kg > 0.0);
  REQUIRE(trip_kg <= aircraft.takeoff_weight_kg);

  const Report report = assess(flight, aircraft, Reserves());
  REQUIRE(report.error.empty());
  REQUIRE(std::isfinite(report.required_kg));
  REQUIRE(report.status == Status::Insufficient);
}

TEST_CASE("FR-04 stores raise the fuel flow by their percentage", "[FR-04]") {
  Aircraft stores = example_aircraft();
  stores.stores_burn_pct = 8;
  REQUIRE_THAT(fuel_flow_kg_h(stores), WithinAbs(540.0, 1e-9));  // 500 x 1.08
}

TEST_CASE("FR-04 stores increase the trip fuel", "[FR-04]") {
  Aircraft stores = example_aircraft();
  stores.stores_burn_pct = 8;
  // 540 kg/h for 600 s = 90 kg, then 9,910 kg -> 89.19 kg, then 9,820.81 kg -> 88.3873 kg
  REQUIRE_THAT(trip_fuel_kg(example_flight(), stores), WithinAbs(267.5773, 1e-4));
  REQUIRE(trip_fuel_kg(example_flight(), stores) > trip_fuel_kg(example_flight(), example_aircraft()));
}
