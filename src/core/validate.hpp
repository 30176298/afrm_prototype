#pragma once

#include <string>

#include "core/types.hpp"

// Checks the inputs - returns an empty string on success or a message describing the first problem found
std::string validate(const Flight& flight, const Aircraft& aircraft, const Reserves& reserves);
