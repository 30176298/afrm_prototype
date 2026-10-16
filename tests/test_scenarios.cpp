#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <chrono>
#include <fstream>
#include <string>

#include "core/fuel.hpp"
#include "core/load.hpp"

using Catch::Matchers::WithinAbs;

namespace {
// Loads a flight and an aircraft from the datasets and assesses them using default reserve profile (5% contingency, 100 NM alternate, 30 min final reserve)
Report assess_files(const char* flight_file, const char* aircraft_file) {
  const std::string dir = std::string(AFRM_DATA_DIR) + "/";
  std::ifstream flight_in(dir + flight_file);
  std::ifstream aircraft_in(dir + aircraft_file);
  REQUIRE(flight_in.is_open());
  REQUIRE(aircraft_in.is_open());

  Flight flight;
  Aircraft aircraft;
  REQUIRE(load_flight(flight_in, flight).empty());
  REQUIRE(load_aircraft(aircraft_in, aircraft).empty());
  return assess(flight, aircraft, Reserves());
}
}  // namespace

// Expected values come from independent calculation

TEST_CASE("FR-06 short flight is feasible", "[FR-06][scenario]") {
  const Report report = assess_files("flight_short.csv", "aircraft_clean.csv");
  REQUIRE(report.error.empty());
  REQUIRE(report.status == Status::Feasible);
  REQUIRE_THAT(report.trip_kg, WithinAbs(1185.6, 0.5));
  REQUIRE_THAT(report.required_kg, WithinAbs(2318.5, 0.5));
}

TEST_CASE("FR-06 medium flight is marginal", "[FR-06][scenario]") {
  const Report report = assess_files("flight_medium.csv", "aircraft_clean.csv");
  REQUIRE(report.status == Status::Marginal);
  REQUIRE_THAT(report.required_kg, WithinAbs(3896.6, 0.5));
  REQUIRE_THAT(report.margin_kg, WithinAbs(103.4, 0.5));
  REQUIRE_THAT(report.max_range_nm, WithinAbs(834.2, 0.5));
}

TEST_CASE("FR-06 long flight is insufficient", "[FR-06][scenario]") {
  const Report report = assess_files("flight_long.csv", "aircraft_clean.csv");
  REQUIRE(report.status == Status::Insufficient);
  REQUIRE_THAT(report.margin_kg, WithinAbs(-844.4, 0.5));
}

TEST_CASE("FR-04 the same medium flight with stores becomes insufficient", "[FR-04][scenario]") {
  const Report clean = assess_files("flight_medium.csv", "aircraft_clean.csv");
  const Report stores = assess_files("flight_medium.csv", "aircraft_stores.csv");
  REQUIRE(clean.status == Status::Marginal);
  REQUIRE(stores.status == Status::Insufficient);
  REQUIRE_THAT(stores.required_kg, WithinAbs(4274.8, 0.5));
  REQUIRE(stores.max_range_nm < clean.max_range_nm);
}

TEST_CASE("NFR-02 a 10,000-row flight is assessed in under 100 ms", "[NFR-02]") {
  Flight flight;
  for (int i = 0; i < 10000; ++i) {
    const double time_s = 60.0 * i;
    flight.push_back({time_s, 450.0, 35000.0, 7.5 * i});  // 450 kt = 7.5 NM per minute
  }
  Aircraft aircraft = {"BIG", 90000, 380000, 80000, 0, 36};

  const auto start = std::chrono::steady_clock::now();
  const Report report = assess(flight, aircraft, Reserves());
  const auto elapsed = std::chrono::steady_clock::now() - start;

  REQUIRE(report.error.empty());
  REQUIRE(elapsed < std::chrono::milliseconds(100));
}
