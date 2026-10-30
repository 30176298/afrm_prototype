#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "core/fuel.hpp"
#include "helpers.hpp"

using Catch::Matchers::ContainsSubstring;
using Catch::Matchers::WithinAbs;

namespace {
// Reserves used with the worked example - 5% contingency, 50 NM alternate, 30 min final.
// The taxi allowance is set to zero so the hand calculations isolate the fuel model, FR-09 tests set it explicitly.
Reserves example_reserves() {
  Reserves reserves;
  reserves.alternate_nm = 50;
  reserves.taxi_kg = 0;
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
  REQUIRE_THAT(report.trip_kg, WithinAbs(247.9225, 1e-4));
  REQUIRE_THAT(report.contingency_kg, WithinAbs(12.3961, 1e-4));             // 5% of 247.9225
  REQUIRE_THAT(report.alternate_kg, WithinAbs(101.5841, 1e-4));              // 487.604 kg/h x 50 NM / 240 kt
  REQUIRE_THAT(report.final_reserve_kg, WithinAbs(243.8019, 1e-4));          // 487.604 kg/h for half an hour
  REQUIRE_THAT(report.required_kg, WithinAbs(605.7047, 1e-4));
  REQUIRE_THAT(report.margin_kg, WithinAbs(94.2953, 1e-4));                  // 700 - 605.7047
}

TEST_CASE("FR-05 the alternate and final reserve are based on the landing weight", "[FR-05]") {
  const Aircraft aircraft = example_aircraft();
  const Report report = assess(example_flight(), aircraft, example_reserves());
  const double flow_kg_h = fuel_flow_kg_h(aircraft, aircraft.takeoff_weight_kg - report.trip_kg);
  REQUIRE_THAT(report.final_reserve_kg, WithinAbs(flow_kg_h * 0.5, 1e-9));
  REQUIRE(report.final_reserve_kg < fuel_flow_kg_h(aircraft) * 0.5);  // lighter than at take-off
  // The trip averages 240 kt (120 NM in half an hour), so 50 NM takes 50 / 240 h
  REQUIRE_THAT(report.alternate_kg, WithinAbs(flow_kg_h * 50.0 / 240.0, 1e-9));
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
  reserves.taxi_kg = 0;
  const Report report = assess(example_flight(), example_aircraft(), reserves);
  REQUIRE_THAT(report.required_kg, WithinAbs(report.trip_kg, 1e-9));
}

TEST_CASE("FR-06 classifies Feasible, Marginal and Insufficient", "[FR-06]") {
  // Required fuel is 605.705 kg and the marginal band is 5% of that, 30.285 kg.
  REQUIRE(assess_with_fuel(700).status == Status::Feasible);      // spare 94.3 kg
  REQUIRE(assess_with_fuel(625).status == Status::Marginal);      // spare 19.3 kg
  REQUIRE(assess_with_fuel(600).status == Status::Insufficient);  // short by 5.7 kg
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
  // range = (fuel - final reserve - alternate) / 1.05 / (trip fuel / 120 NM), where
  // trip fuel = 247.9225 kg, final reserve = 243.8019 kg and alternate = 101.5841 kg
  REQUIRE_THAT(assess_with_fuel(700).max_range_nm, WithinAbs(163.4677, 1e-3));
  REQUIRE_THAT(assess_with_fuel(625).max_range_nm, WithinAbs(128.8946, 1e-3));
  REQUIRE_THAT(assess_with_fuel(600).max_range_nm, WithinAbs(117.3703, 1e-3));
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
  REQUIRE_THAT(loaded.trip_kg, WithinAbs(267.5773, 1e-4));
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

TEST_CASE("FR-09 the taxi allowance is added to the required fuel", "[FR-09]") {
  Reserves reserves = example_reserves();
  reserves.taxi_kg = 150;
  const Report report = assess(example_flight(), example_aircraft(), reserves);
  REQUIRE_THAT(report.taxi_kg, WithinAbs(150.0, 1e-9));
  REQUIRE_THAT(report.required_kg, WithinAbs(755.7047, 1e-4));  // 605.7047 + 150
  REQUIRE_THAT(report.margin_kg, WithinAbs(-55.7047, 1e-4));    // 700 - 755.7047
  REQUIRE(report.status == Status::Insufficient);
}

TEST_CASE("FR-09 the taxi allowance does not change the trip or the reserves", "[FR-09]") {
  Reserves reserves = example_reserves();
  const Report without = assess(example_flight(), example_aircraft(), reserves);
  reserves.taxi_kg = 150;
  const Report with = assess(example_flight(), example_aircraft(), reserves);
  REQUIRE_THAT(with.trip_kg, WithinAbs(without.trip_kg, 1e-9));
  REQUIRE_THAT(with.contingency_kg, WithinAbs(without.contingency_kg, 1e-9));
  REQUIRE_THAT(with.alternate_kg, WithinAbs(without.alternate_kg, 1e-9));
  REQUIRE_THAT(with.final_reserve_kg, WithinAbs(without.final_reserve_kg, 1e-9));
}

TEST_CASE("FR-09 the taxi allowance reduces the maximum range", "[FR-09]") {
  Reserves reserves = example_reserves();
  reserves.taxi_kg = 150;
  REQUIRE_THAT(assess(example_flight(), example_aircraft(), reserves).max_range_nm, WithinAbs(94.3216, 1e-3));
}

TEST_CASE("FR-09 the default taxi allowance is 100 kg", "[FR-09]") {
  REQUIRE_THAT(Reserves().taxi_kg, WithinAbs(100.0, 1e-9));
}
