add_executable(afrm main.cpp)
target_link_libraries(afrm PRIVATE afrm_core afrm_options imgui)

target_compile_definitions(afrm PRIVATE AFRM_DATA_DIR="${PROJECT_SOURCE_DIR}/data")
