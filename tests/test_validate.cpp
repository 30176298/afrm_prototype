#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <limits>

#include "core/validate.hpp"
#include "helpers.hpp"

using Catch::Matchers::ContainsSubstring;

namespace {
std::string check(const Flight& flight, const Aircraft& aircraft, const Reserves& reserves = Reserves()) {
  return validate(flight, aircraft, reserves);
}
}  // namespace

TEST_CASE("DR-03 accepts the worked example", "[DR-03]") {
  REQUIRE(check(example_flight(), example_aircraft()).empty());
}

TEST_CASE("DR-03 rejects flight values outside the limits and names them", "[DR-03]") {
  Flight flight = example_flight();
  flight[2].speed_kt = 800;
  REQUIRE_THAT(check(flight, example_aircraft()), ContainsSubstring("speed_kt (row 3)"));

  flight = example_flight();
  flight[1].altitude_ft = 60000;
  REQUIRE_THAT(check(flight, example_aircraft()), ContainsSubstring("altitude_ft (row 2)"));

  flight = example_flight();
  flight[0].altitude_ft = -10;
  REQUIRE_THAT(check(flight, example_aircraft()), ContainsSubstring("altitude_ft (row 1)"));
}

TEST_CASE("DR-03 accepts values exactly on a limit", "[DR-03]") {
  Flight flight = example_flight();
  flight[1].speed_kt = 700;
  flight[1].altitude_ft = 45000;
  Aircraft aircraft = example_aircraft();
  aircraft.stores_burn_pct = 50;
  REQUIRE(check(flight, aircraft).empty());
}

TEST_CASE("DR-03 rejects aircraft values outside the limits", "[DR-03]") {
  Aircraft aircraft = example_aircraft();
  aircraft.burn_kg_h_per_t = 0;
  REQUIRE_THAT(check(example_flight(), aircraft), ContainsSubstring("burn_kg_h_per_t = 0"));

  aircraft = example_aircraft();
  aircraft.stores_burn_pct = 80;
  REQUIRE_THAT(check(example_flight(), aircraft), ContainsSubstring("stores_burn_pct"));

  aircraft = example_aircraft();
  aircraft.takeoff_weight_kg = 10;
  REQUIRE_THAT(check(example_flight(), aircraft), ContainsSubstring("takeoff_weight_kg"));
}

TEST_CASE("DR-03 rejects reserve settings outside the limits", "[DR-03]") {
  Reserves reserves;
  reserves.contingency_pct = 150;
  REQUIRE_THAT(check(example_flight(), example_aircraft(), reserves), ContainsSubstring("contingency_pct"));

  reserves = Reserves();
  reserves.final_reserve_min = -5;
  REQUIRE_THAT(check(example_flight(), example_aircraft(), reserves), ContainsSubstring("final_reserve_min"));
}

TEST_CASE("DR-03 rejects a NaN", "[DR-03]") {
  Aircraft aircraft = example_aircraft();
  aircraft.burn_kg_h_per_t = std::numeric_limits<double>::quiet_NaN();
  REQUIRE_FALSE(check(example_flight(), aircraft).empty());

  Flight flight = example_flight();
  flight[2].speed_kt = std::numeric_limits<double>::quiet_NaN();
  REQUIRE_FALSE(check(flight, example_aircraft()).empty());

  flight = example_flight();
  flight[2].time_s = std::numeric_limits<double>::quiet_NaN();
  REQUIRE_FALSE(check(flight, example_aircraft()).empty());
}

TEST_CASE("DR-04 rejects a flight with fewer than two rows", "[DR-04]") {
  REQUIRE_THAT(check(Flight(), example_aircraft()), ContainsSubstring("at least 2 rows"));
  REQUIRE_THAT(check(Flight{{0, 240, 0, 0}}, example_aircraft()), ContainsSubstring("at least 2 rows"));
}

TEST_CASE("DR-04 rejects time that does not increase", "[DR-04]") {
  Flight flight = example_flight();
  flight[2].time_s = 600;  // same as the row before
  REQUIRE_THAT(check(flight, example_aircraft()), ContainsSubstring("time_s (row 3)"));

  flight = example_flight();
  flight[2].time_s = 300;  // goes backwards
  REQUIRE_THAT(check(flight, example_aircraft()), ContainsSubstring("time_s (row 3)"));
}

TEST_CASE("DR-04 rejects distance that decreases", "[DR-04]") {
  Flight flight = example_flight();
  flight[2].distance_nm = 30;
  REQUIRE_THAT(check(flight, example_aircraft()), ContainsSubstring("distance_nm (row 3)"));
}

TEST_CASE("DR-04 rejects a flight that covers no distance", "[DR-04]") {
  Flight flight = example_flight();
  for (FlightPoint& point : flight) point.distance_nm = 0;
  REQUIRE_THAT(check(flight, example_aircraft()), ContainsSubstring("positive distance"));
}

TEST_CASE("DR-05 rejects a missing model name", "[DR-05]") {
  Aircraft aircraft = example_aircraft();
  aircraft.model = "";
  REQUIRE_THAT(check(example_flight(), aircraft), ContainsSubstring("model name"));
}

TEST_CASE("DR-05 rejects fuel on board above capacity", "[DR-05]") {
  Aircraft aircraft = example_aircraft();
  aircraft.fuel_onboard_kg = 2500;  // capacity is 2000
  REQUIRE_THAT(check(example_flight(), aircraft), ContainsSubstring("fuel_capacity_kg"));
}

TEST_CASE("DR-05 rejects fuel on board that is not less than the take-off weight", "[DR-05]") {
  Aircraft aircraft = example_aircraft();
  aircraft.fuel_capacity_kg = 20000;
  aircraft.fuel_onboard_kg = 10000;  // equal to take-off weight
  REQUIRE_THAT(check(example_flight(), aircraft), ContainsSubstring("less than takeoff_weight_kg"));
}
