# Test configuration
include(GoogleTest)

# Put all test executables in build/Testing/ directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Testing)

# Unit tests subdirectory (Phase 1 - in progress)
add_subdirectory(unit)

# Integration tests subdirectory (Phase 1.7+)
add_subdirectory(integration)

# Placeholder test to verify testing infrastructure
add_executable(placeholder_test
    ${CMAKE_CURRENT_SOURCE_DIR}/placeholder_test.cpp
)

target_link_libraries(placeholder_test
    gtest_main
    gtest
)

gtest_discover_tests(placeholder_test)

# E2E tests - Prince of Persia complete validation
find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
  add_test(
    NAME e2e_pop_complete_validation
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/e2e/apple2/prince_of_persia/test_pop_complete.py
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  )
  set_tests_properties(e2e_pop_complete_validation PROPERTIES
    LABELS "e2e;prince_of_persia"
    TIMEOUT 120
  )
endif()
