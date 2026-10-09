#include <catch2/catch_test_macros.hpp>
#include <string>

#include "core/version.hpp"

// Toolchain builds the core library and links it into a test.
TEST_CASE("Core library links and reports a version", "[ENV-01]") {
  REQUIRE(std::string(afrm_version()) == "0.1.0");
}
