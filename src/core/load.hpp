#pragma once

#include <istream>
#include <string>

#include "core/types.hpp"

// Both loaders return an empty string on success, or a message describing the first problem found
//Both read from stream for testability
std::string load_flight(std::istream& in, Flight& out);
std::string load_aircraft(std::istream& in, Aircraft& out);
