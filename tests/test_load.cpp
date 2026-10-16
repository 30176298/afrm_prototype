#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <fstream>
#include <sstream>
#include <string>

#include "core/load.hpp"

using Catch::Matchers::ContainsSubstring;

namespace {
const char* kFlightHeader = "time_s,speed_kt,altitude_ft,distance_nm\n";
const char* kAircraftHeader =
    "model,fuel_capacity_kg,takeoff_weight_kg,fuel_onboard_kg,stores_burn_pct,burn_kg_h_per_t\n";

std::string flight_error(const std::string& text) {
  std::istringstream in(text);
  Flight flight;
  return load_flight(in, flight);
}

std::string aircraft_error(const std::string& text) {
  std::istringstream in(text);
  Aircraft aircraft;
  return load_aircraft(in, aircraft);
}
}  // namespace

TEST_CASE("FR-01 loads a flight profile", "[FR-01]") {
  std::istringstream in(std::string(kFlightHeader) + "0,150,0,0\n60,172.5,2000,2.4\n");
  Flight flight;
  REQUIRE(load_flight(in, flight).empty());
  REQUIRE(flight.size() == 2);
  REQUIRE(flight[1].time_s == 60);
  REQUIRE(flight[1].speed_kt == 172.5);
  REQUIRE(flight[1].altitude_ft == 2000);
  REQUIRE(flight[1].distance_nm == 2.4);
}

TEST_CASE("FR-01 copes with Windows line endings, a byte order mark and blank lines", "[FR-01]") {
  const std::string text = "\xEF\xBB\xBF" "time_s,speed_kt,altitude_ft,distance_nm\r\n"
                           "0,150,0,0\r\n\r\n60,172.5,2000,2.4\r\n\r\n";
  std::istringstream in(text);
  Flight flight;
  REQUIRE(load_flight(in, flight).empty());
  REQUIRE(flight.size() == 2);
}

TEST_CASE("FR-01 loads the shipped flight files", "[FR-01]") {
  for (const char* name : {"flight_short.csv", "flight_medium.csv", "flight_long.csv"}) {
    std::ifstream file(std::string(AFRM_DATA_DIR) + "/" + name);
    REQUIRE(file.is_open());
    Flight flight;
    REQUIRE(load_flight(file, flight).empty());
    REQUIRE(flight.size() > 50);
    REQUIRE(flight.front().time_s == 0);
  }
}

TEST_CASE("FR-02 loads aircraft data", "[FR-02]") {
  std::istringstream in(std::string(kAircraftHeader) + "AFRM-100,9000,38000,4000,8,36\n");
  Aircraft aircraft;
  REQUIRE(load_aircraft(in, aircraft).empty());
  REQUIRE(aircraft.model == "AFRM-100");
  REQUIRE(aircraft.fuel_capacity_kg == 9000);
  REQUIRE(aircraft.takeoff_weight_kg == 38000);
  REQUIRE(aircraft.fuel_onboard_kg == 4000);
  REQUIRE(aircraft.stores_burn_pct == 8);
  REQUIRE(aircraft.burn_kg_h_per_t == 36);
}

TEST_CASE("FR-02 loads the shipped aircraft files", "[FR-02]") {
  for (const char* name : {"aircraft_clean.csv", "aircraft_stores.csv"}) {
    std::ifstream file(std::string(AFRM_DATA_DIR) + "/" + name);
    REQUIRE(file.is_open());
    Aircraft aircraft;
    REQUIRE(load_aircraft(file, aircraft).empty());
    REQUIRE(aircraft.fuel_onboard_kg == 4000);
  }
}

TEST_CASE("DR-01 rejects headings with the wrong units", "[DR-01]") {
  REQUIRE_THAT(flight_error("time_s,speed_kmh,altitude_ft,distance_nm\n0,150,0,0\n"),
               ContainsSubstring("unexpected columns or units"));
  REQUIRE_THAT(flight_error("time_min,speed_kt,altitude_ft,distance_nm\n0,150,0,0\n"),
               ContainsSubstring("unexpected columns or units"));
  REQUIRE_THAT(aircraft_error("model,fuel_capacity_lb,takeoff_weight_kg,fuel_onboard_kg,"
                              "stores_burn_pct,burn_kg_h_per_t\nX,9000,38000,4000,8,36\n"),
               ContainsSubstring("unexpected columns or units"));
}

TEST_CASE("DR-02 rejects empty files and missing data", "[DR-02]") {
  REQUIRE_THAT(flight_error(""), ContainsSubstring("empty"));
  REQUIRE_THAT(aircraft_error("\n\n"), ContainsSubstring("empty"));
  REQUIRE_THAT(aircraft_error(kAircraftHeader), ContainsSubstring("no aircraft data"));
}

TEST_CASE("DR-02 names the line of a malformed flight row", "[DR-02]") {
  const std::string head = kFlightHeader;
  REQUIRE_THAT(flight_error(head + "0,150,0,0\n60,abc,2000,2.4\n"), ContainsSubstring("line 3"));
  REQUIRE_THAT(flight_error(head + "0,150,0\n"), ContainsSubstring("line 2"));
  REQUIRE_THAT(flight_error(head + "0,150,0,0,9\n"), ContainsSubstring("line 2"));
  REQUIRE_THAT(flight_error(head + "0,150,0,0,\n"), ContainsSubstring("line 2"));
  REQUIRE_THAT(flight_error(head + "0,150,,0\n"), ContainsSubstring("line 2"));
}

TEST_CASE("DR-02 rejects non-finite numbers", "[DR-02]") {
  const std::string head = kFlightHeader;
  REQUIRE_THAT(flight_error(head + "0,nan,0,0\n"), ContainsSubstring("line 2"));
  REQUIRE_THAT(flight_error(head + "0,inf,0,0\n"), ContainsSubstring("line 2"));
  REQUIRE_THAT(flight_error(head + "0,1e999,0,0\n"), ContainsSubstring("line 2"));
}

TEST_CASE("DR-02 rejects a malformed aircraft row", "[DR-02]") {
  const std::string head = kAircraftHeader;
  REQUIRE_THAT(aircraft_error(head + "X,9000,38000\n"), ContainsSubstring("line 2"));
  REQUIRE_THAT(aircraft_error(head + "X,9000,38000,four thousand,8,36\n"),
               ContainsSubstring("four thousand"));
}

TEST_CASE("DR-02 leaves the output untouched when loading fails", "[DR-02]") {
  Flight flight = {{0, 150, 0, 0}};
  std::istringstream in(std::string(kFlightHeader) + "0,150,0,0\n60,abc,2000,2.4\n");
  REQUIRE_FALSE(load_flight(in, flight).empty());
  REQUIRE(flight.size() == 1);
}
