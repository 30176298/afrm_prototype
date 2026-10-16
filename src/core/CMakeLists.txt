add_library(afrm_core STATIC
  version.cpp
  load.cpp
  validate.cpp
  fuel.cpp)
target_include_directories(afrm_core PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_compile_definitions(afrm_core PUBLIC AFRM_VERSION="${PROJECT_VERSION}")
target_link_libraries(afrm_core PRIVATE afrm_options)
