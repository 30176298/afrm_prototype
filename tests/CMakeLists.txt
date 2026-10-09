include(Catch)

add_executable(afrm_tests test_smoke.cpp)
target_link_libraries(afrm_tests PRIVATE afrm_core afrm_options Catch2::Catch2WithMain)

# Registers every test case with CTest
catch_discover_tests(afrm_tests)
