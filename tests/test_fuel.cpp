#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/fuel.hpp"
#include "helpers.hpp"

using Catch::Matchers::WithinAbs;

TEST_CASE("FR-03 fuel flow matches the hand calculation", "[FR-03]") {
  // 50 kg/h per tonne x 10 tonnes = 500 kg/h
  REQUIRE_THAT(fuel_flow_kg_h(example_aircraft()), WithinAbs(500.0, 1e-9));
}

TEST_CASE("FR-03 trip fuel matches the hand calculation", "[FR-03]") {
  // 500 kg/h for 1800 s (half an hour) = 250 kg
  REQUIRE_THAT(trip_fuel_kg(example_flight(), example_aircraft()), WithinAbs(250.0, 1e-9));
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
  REQUIRE_THAT(trip_fuel_kg(example_flight(), heavy), WithinAbs(275.0, 1e-9));  // 550 kg/h for 0.5 h
}

TEST_CASE("FR-04 stores raise the fuel flow by their percentage", "[FR-04]") {
  Aircraft stores = example_aircraft();
  stores.stores_burn_pct = 8;
  REQUIRE_THAT(fuel_flow_kg_h(stores), WithinAbs(540.0, 1e-9));  // 500 x 1.08
}

TEST_CASE("FR-04 stores increase the trip fuel", "[FR-04]") {
  Aircraft stores = example_aircraft();
  stores.stores_burn_pct = 8;
  REQUIRE_THAT(trip_fuel_kg(example_flight(), stores), WithinAbs(270.0, 1e-9));  // 540 kg/h for 0.5 h
}
