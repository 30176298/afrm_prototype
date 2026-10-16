#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "core/fuel.hpp"
#include "helpers.hpp"

using Catch::Matchers::ContainsSubstring;
using Catch::Matchers::WithinAbs;

namespace {
// Reserves used with the worked example - 5% contingency, 50 NM alternate, 30 min final
Reserves example_reserves() {
  Reserves reserves;
  reserves.alternate_nm = 50;
  return reserves;
}

Report assess_with_fuel(double fuel_onboard_kg) {
  Aircraft aircraft = example_aircraft();
  aircraft.fuel_onboard_kg = fuel_onboard_kg;
  return assess(example_flight(), aircraft, example_reserves());
}
}  // namespace

TEST_CASE("FR-05 reserves and required fuel match the hand calculation", "[FR-05]") {
  const Report report = assess(example_flight(), example_aircraft(), example_reserves());
  REQUIRE(report.error.empty());
  REQUIRE_THAT(report.trip_kg, WithinAbs(250.0, 1e-9));
  REQUIRE_THAT(report.contingency_kg, WithinAbs(12.5, 1e-9));                // 5% of 250
  REQUIRE_THAT(report.alternate_kg, WithinAbs(104.1667, 1e-4));              // 50 NM x (250 / 120) kg/NM
  REQUIRE_THAT(report.final_reserve_kg, WithinAbs(250.0, 1e-9));             // 500 kg/h for half an hour
  REQUIRE_THAT(report.required_kg, WithinAbs(616.6667, 1e-4));
  REQUIRE_THAT(report.margin_kg, WithinAbs(83.3333, 1e-4));                  // 700 - 616.6667
}

TEST_CASE("FR-05 the reserves follow the user's settings", "[FR-05]") {
  Reserves reserves = example_reserves();
  const Report base = assess(example_flight(), example_aircraft(), reserves);

  reserves.contingency_pct = 10;
  REQUIRE_THAT(assess(example_flight(), example_aircraft(), reserves).contingency_kg,
               WithinAbs(2 * base.contingency_kg, 1e-9));

  reserves = example_reserves();
  reserves.alternate_nm = 100;
  REQUIRE_THAT(assess(example_flight(), example_aircraft(), reserves).alternate_kg,
               WithinAbs(2 * base.alternate_kg, 1e-9));

  reserves = example_reserves();
  reserves.final_reserve_min = 60;
  REQUIRE_THAT(assess(example_flight(), example_aircraft(), reserves).final_reserve_kg,
               WithinAbs(2 * base.final_reserve_kg, 1e-9));
}

TEST_CASE("FR-05 zero reserves leave only the trip fuel", "[FR-05]") {
  Reserves reserves;
  reserves.contingency_pct = 0;
  reserves.alternate_nm = 0;
  reserves.final_reserve_min = 0;
  const Report report = assess(example_flight(), example_aircraft(), reserves);
  REQUIRE_THAT(report.required_kg, WithinAbs(report.trip_kg, 1e-9));
}

TEST_CASE("FR-06 classifies Feasible, Marginal and Insufficient", "[FR-06]") {
  // Required fuel is 616.667 kg and the marginal band is 5% of that, 30.833 kg.
  REQUIRE(assess_with_fuel(700).status == Status::Feasible);      // spare 83.3 kg
  REQUIRE(assess_with_fuel(640).status == Status::Marginal);      // spare 23.3 kg
  REQUIRE(assess_with_fuel(600).status == Status::Insufficient);  // short by 16.7 kg
}

TEST_CASE("FR-06 classification boundaries", "[FR-06]") {
  const double required = assess_with_fuel(700).required_kg;
  const double band = required * 0.05;

  REQUIRE(assess_with_fuel(required - 0.01).status == Status::Insufficient);
  REQUIRE(assess_with_fuel(required).status == Status::Marginal);  // exactly enough
  REQUIRE(assess_with_fuel(required + band - 0.01).status == Status::Marginal);
  REQUIRE(assess_with_fuel(required + band + 0.01).status == Status::Feasible);
}

TEST_CASE("FR-06 status names", "[FR-06]") {
  REQUIRE(std::string(status_text(Status::Feasible)) == "Feasible");
  REQUIRE(std::string(status_text(Status::Marginal)) == "Marginal");
  REQUIRE(std::string(status_text(Status::Insufficient)) == "Insufficient");
}

TEST_CASE("FR-07 maximum range matches the hand calculation", "[FR-07]") {
  // range = ((fuel - 250) / (250 / 120) - 50) / 1.05
  REQUIRE_THAT(assess_with_fuel(700).max_range_nm, WithinAbs(158.0952, 1e-3));
  REQUIRE_THAT(assess_with_fuel(640).max_range_nm, WithinAbs(130.6667, 1e-3));
  REQUIRE_THAT(assess_with_fuel(600).max_range_nm, WithinAbs(112.3810, 1e-3));
}

TEST_CASE("FR-07 with exactly the required fuel the range equals the planned distance", "[FR-07]") {
  const double required = assess_with_fuel(700).required_kg;
  REQUIRE_THAT(assess_with_fuel(required).max_range_nm, WithinAbs(120.0, 1e-6));
}

TEST_CASE("FR-07 more fuel gives more range, and range is never negative", "[FR-07]") {
  REQUIRE(assess_with_fuel(900).max_range_nm > assess_with_fuel(700).max_range_nm);
  REQUIRE(assess_with_fuel(100).max_range_nm == 0.0);  // less than the final reserve alone
}

TEST_CASE("FR-04 stores increase the required fuel and reduce the range", "[FR-04]") {
  Aircraft stores = example_aircraft();
  stores.stores_burn_pct = 8;
  const Report clean = assess(example_flight(), example_aircraft(), example_reserves());
  const Report loaded = assess(example_flight(), stores, example_reserves());
  REQUIRE_THAT(loaded.trip_kg, WithinAbs(270.0, 1e-9));
  REQUIRE(loaded.required_kg > clean.required_kg);
  REQUIRE(loaded.max_range_nm < clean.max_range_nm);
}

TEST_CASE("DR-06 invalid input is reported and nothing is calculated", "[DR-06]") {
  Flight flight = example_flight();
  flight[2].speed_kt = 900;
  const Report report = assess(flight, example_aircraft(), example_reserves());
  REQUIRE_THAT(report.error, ContainsSubstring("speed_kt (row 3)"));
  REQUIRE(report.trip_kg == 0.0);
  REQUIRE(report.required_kg == 0.0);
  REQUIRE(report.max_range_nm == 0.0);
}

TEST_CASE("DR-06 invalid reserve settings are reported and nothing is calculated", "[DR-06]") {
  Reserves reserves = example_reserves();
  reserves.contingency_pct = -1;
  const Report report = assess(example_flight(), example_aircraft(), reserves);
  REQUIRE_THAT(report.error, ContainsSubstring("contingency_pct"));
  REQUIRE(report.required_kg == 0.0);
}
