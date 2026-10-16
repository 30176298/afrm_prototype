#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <fstream>
#include <string>

#include "core/fuel.hpp"
#include "core/load.hpp"

using Catch::Matchers::ContainsSubstring;

namespace {
std::string path(const char* name) { return std::string(AFRM_DATA_DIR) + "/invalid/" + name; }
}  // namespace

TEST_CASE("DR-01 the shipped wrong-unit flight file is rejected", "[DR-01][invalid-file]") {
  std::ifstream file(path("flight_wrong_units.csv"));
  REQUIRE(file.is_open());
  Flight flight;
  REQUIRE_THAT(load_flight(file, flight), ContainsSubstring("unexpected columns or units"));
}

TEST_CASE("DR-01 the shipped wrong-unit aircraft file is rejected", "[DR-01][invalid-file]") {
  std::ifstream file(path("aircraft_wrong_units.csv"));
  REQUIRE(file.is_open());
  Aircraft aircraft;
  REQUIRE_THAT(load_aircraft(file, aircraft), ContainsSubstring("unexpected columns or units"));
}

TEST_CASE("DR-02 the shipped malformed flight file is rejected and the line is named", "[DR-02][invalid-file]") {
  std::ifstream file(path("flight_bad_row.csv"));
  REQUIRE(file.is_open());
  Flight flight;
  REQUIRE_THAT(load_flight(file, flight), ContainsSubstring("line 3"));
}

TEST_CASE("DR-05 the shipped overfilled aircraft loads but the assessment rejects it", "[DR-05][DR-06][invalid-file]") {
  std::ifstream flight_file(std::string(AFRM_DATA_DIR) + "/flight_short.csv");
  std::ifstream aircraft_file(path("aircraft_overfilled.csv"));
  REQUIRE(flight_file.is_open());
  REQUIRE(aircraft_file.is_open());
  Flight flight;
  Aircraft aircraft;
  REQUIRE(load_flight(flight_file, flight).empty());
  REQUIRE(load_aircraft(aircraft_file, aircraft).empty());

  const Report report = assess(flight, aircraft, Reserves());
  REQUIRE_THAT(report.error, ContainsSubstring("fuel_capacity_kg"));
  REQUIRE(report.required_kg == 0.0);
}
