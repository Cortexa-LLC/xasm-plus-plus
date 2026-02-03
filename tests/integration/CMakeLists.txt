# Integration tests - end-to-end assembly pipeline validation

# Binary compatibility tests (compare with vasm-ext)
add_subdirectory(binary_compat)

# Test programs (simple syntax)
set(TEST_PROGRAMS
  01_hello
  02_labels
  03_data
  04_org
  05_comments
)

# Test programs (merlin syntax - Prince of Persia validation)
set(MERLIN_TEST_PROGRAMS
  06_pop_eq
  07_pop_subs
  09_xc_directive
)

# For each test program:
# 1. Run xasm++ to assemble .asm -> .bin
# 2. Compare output with expected golden file
foreach(test_name ${TEST_PROGRAMS})
  # Assemble test program
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${test_name}.bin
    COMMAND $<TARGET_FILE:xasm++>
            ${CMAKE_CURRENT_SOURCE_DIR}/${test_name}.asm
            --output ${CMAKE_CURRENT_BINARY_DIR}/${test_name}.bin
    DEPENDS xasm++ ${CMAKE_CURRENT_SOURCE_DIR}/${test_name}.asm
    COMMENT "Assembling ${test_name}.asm"
  )

  # Add to build
  add_custom_target(integration_${test_name}_build
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${test_name}.bin
  )

  # Compare with expected output
  add_test(
    NAME integration_${test_name}
    COMMAND ${CMAKE_COMMAND} -E compare_files
            ${CMAKE_CURRENT_BINARY_DIR}/${test_name}.bin
            ${CMAKE_CURRENT_SOURCE_DIR}/expected/${test_name}.bin
  )

  # Make test depend on build
  set_tests_properties(integration_${test_name} PROPERTIES
    FIXTURES_REQUIRED integration_${test_name}_fixture
  )

  add_test(
    NAME integration_${test_name}_setup
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target integration_${test_name}_build
  )

  set_tests_properties(integration_${test_name}_setup PROPERTIES
    FIXTURES_SETUP integration_${test_name}_fixture
  )
endforeach()

# Merlin syntax tests (Prince of Persia validation)
foreach(test_name ${MERLIN_TEST_PROGRAMS})
  # Assemble test program with Merlin syntax
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${test_name}.bin
    COMMAND $<TARGET_FILE:xasm++>
            --syntax merlin
            --cpu 6502
            ${CMAKE_CURRENT_SOURCE_DIR}/${test_name}.asm
            --output ${CMAKE_CURRENT_BINARY_DIR}/${test_name}.bin
    DEPENDS xasm++ ${CMAKE_CURRENT_SOURCE_DIR}/${test_name}.asm
    COMMENT "Assembling ${test_name}.asm (Merlin syntax)"
  )

  # Add to build
  add_custom_target(integration_${test_name}_build
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${test_name}.bin
  )

  # Compare with expected output
  add_test(
    NAME integration_${test_name}
    COMMAND ${CMAKE_COMMAND} -E compare_files
            ${CMAKE_CURRENT_BINARY_DIR}/${test_name}.bin
            ${CMAKE_CURRENT_SOURCE_DIR}/${test_name}.bin
  )

  # Make test depend on build
  set_tests_properties(integration_${test_name} PROPERTIES
    FIXTURES_REQUIRED integration_${test_name}_fixture
  )

  add_test(
    NAME integration_${test_name}_setup
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target integration_${test_name}_build
  )

  set_tests_properties(integration_${test_name}_setup PROPERTIES
    FIXTURES_SETUP integration_${test_name}_fixture
  )
endforeach()
