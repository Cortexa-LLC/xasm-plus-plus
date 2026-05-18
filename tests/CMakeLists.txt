# Test configuration
include(GoogleTest)

# Helper: add a GTest unit test executable.
#
# Usage: xasm_add_unit_test(<target>
#          [SOURCE src.cpp]        # primary source; defaults to <target>.cpp
#          [LIBS lib ...]          # link libraries (besides gtest_main gtest)
#          [EXTRA_SOURCES src ...] # additional source files
#          [INCLUDE_DIRS dir ...]) # additional private include directories
#
# Adding a new test: call this once, link against the library your .cpp uses.
# Library quick-reference: core→xasm_core  cpu→xasm_cpu
#                          syntax→xasm_syntax  output→xasm_output
function(xasm_add_unit_test name)
    cmake_parse_arguments(ARG "" "SOURCE" "LIBS;EXTRA_SOURCES;INCLUDE_DIRS" ${ARGN})
    if(ARG_SOURCE)
        set(_primary ${ARG_SOURCE})
    else()
        set(_primary ${name}.cpp)
    endif()
    add_executable(${name} ${_primary} ${ARG_EXTRA_SOURCES})
    target_link_libraries(${name} PRIVATE ${ARG_LIBS} gtest_main gtest)
    if(ARG_INCLUDE_DIRS)
        target_include_directories(${name} PRIVATE ${ARG_INCLUDE_DIRS})
    endif()
    gtest_discover_tests(${name})
endfunction()

# Put all test executables in build/Testing/ directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Testing)

# Unit tests subdirectory (Phase 1 - in progress)
add_subdirectory(unit)

# Integration tests subdirectory (Phase 1.7+)
add_subdirectory(integration)

# Placeholder test to verify testing infrastructure
xasm_add_unit_test(placeholder_test)

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
