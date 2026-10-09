# Third-party code is downloaded as a release archive and checked against a
# SHA-256 hash, so a changed or tampered download fails the configure step.
include(FetchContent)

FetchContent_Declare(catch2
  URL      https://github.com/catchorg/Catch2/archive/refs/tags/v3.16.0.tar.gz
  URL_HASH SHA256=0957cae5821b17ce07f0833aaa52b5137643a8382203221f363a8303c109af34
  SYSTEM)
FetchContent_MakeAvailable(catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
