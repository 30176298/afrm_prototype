include(Catch)

add_executable(afrm_tests
  test_smoke.cpp
  test_load.cpp
  test_validate.cpp)
target_link_libraries(afrm_tests PRIVATE afrm_core afrm_options Catch2::Catch2WithMain)

target_compile_definitions(afrm_tests PRIVATE AFRM_DATA_DIR="${PROJECT_SOURCE_DIR}/data")

# Registers every test with CTest
catch_discover_tests(afrm_tests)
