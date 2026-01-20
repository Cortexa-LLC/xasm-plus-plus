# Test configuration
include(GoogleTest)

# Unit tests subdirectory (Phase 1 - in progress)
add_subdirectory(unit)

# Integration tests subdirectory (Phase 2+)
# add_subdirectory(integration)

# Placeholder test to verify testing infrastructure
add_executable(placeholder_test
    ${CMAKE_CURRENT_SOURCE_DIR}/placeholder_test.cpp
)

target_link_libraries(placeholder_test
    gtest_main
    gtest
)

gtest_discover_tests(placeholder_test)
