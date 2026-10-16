#include "core/load.hpp"

#include <cmath>
#include <cstdlib>
#include <sstream>
#include <vector>

namespace {

// The headings state the units, so a file in other units is rejected
constexpr const char* kFlightHeader = "time_s,speed_kt,altitude_ft,distance_nm";
constexpr const char* kAircraftHeader =
    "model,fuel_capacity_kg,takeoff_weight_kg,fuel_onboard_kg,stores_burn_pct,burn_kg_h_per_t";

// Reads the next non-blank line, correct windows line endings
bool next_line(std::istream& in, std::string& line, int& line_no) {
  while (std::getline(in, line)) {
    ++line_no;
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (!line.empty()) return true;
  }
  return false;
}

std::vector<std::string> split(const std::string& line) {
  std::vector<std::string> fields;
  std::stringstream stream(line);
  std::string field;
  while (std::getline(stream, field, ',')) fields.push_back(field);
  if (line.back() == ',') fields.push_back("");  // getline drops a trailing empty field
  return fields;
}

// True only if the whole field is a finite number
bool to_number(const std::string& text, double& value) {
  char* end = nullptr;
  value = std::strtod(text.c_str(), &end);
  return end != text.c_str() && *end == '\0' && std::isfinite(value);
}

std::string at_line(int line_no, const std::string& message) {
  return "line " + std::to_string(line_no) + ": " + message;
}

// Reads the first line and checks that it is exactly the expected heading
std::string check_header(std::istream& in, const char* expected, int& line_no) {
  std::string line;
  if (!next_line(in, line, line_no)) return "file is empty";
  if (line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);  // byte order mark check
  if (line != expected) {
    return at_line(line_no, "unexpected columns or units, expected '" + std::string(expected) + "'");
  }
  return "";
}

}  // namespace

std::string load_flight(std::istream& in, Flight& out) {
  int line_no = 0;
  std::string error = check_header(in, kFlightHeader, line_no);
  if (!error.empty()) return error;

  Flight flight;
  std::string line;
  while (next_line(in, line, line_no)) {
    const std::vector<std::string> fields = split(line);
    FlightPoint point{};
    if (fields.size() != 4 || !to_number(fields[0], point.time_s) ||
        !to_number(fields[1], point.speed_kt) || !to_number(fields[2], point.altitude_ft) ||
        !to_number(fields[3], point.distance_nm)) {
      return at_line(line_no, "expected 4 numeric values");
    }
    flight.push_back(point);
  }
  out = flight;
  return "";
}

std::string load_aircraft(std::istream& in, Aircraft& out) {
  int line_no = 0;
  std::string error = check_header(in, kAircraftHeader, line_no);
  if (!error.empty()) return error;

  std::string line;
  if (!next_line(in, line, line_no)) return "no aircraft data after the heading";

  const std::vector<std::string> fields = split(line);
  Aircraft aircraft{};
  double* numbers[] = {&aircraft.fuel_capacity_kg, &aircraft.takeoff_weight_kg,
                       &aircraft.fuel_onboard_kg, &aircraft.stores_burn_pct,
                       &aircraft.burn_kg_h_per_t};
  if (fields.size() != 6) return at_line(line_no, "expected a model name and 5 numeric values");
  for (std::size_t i = 0; i < 5; ++i) {
    if (!to_number(fields[i + 1], *numbers[i])) {
      return at_line(line_no, "'" + fields[i + 1] + "' is not a valid number");
    }
  }
  aircraft.model = fields[0];
  out = aircraft;
  return "";
}
