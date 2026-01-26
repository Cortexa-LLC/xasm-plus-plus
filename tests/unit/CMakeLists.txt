# Unit tests

# Atom tests
add_executable(test_atom
    test_atom.cpp
)

target_link_libraries(test_atom
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_atom)

# Expression tests
add_executable(test_expression
    test_expression.cpp
)

target_link_libraries(test_expression
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_expression)

# Symbol tests
add_executable(test_symbol
    test_symbol.cpp
)

target_link_libraries(test_symbol
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_symbol)

# Section tests
add_executable(test_section
    test_section.cpp
)

target_link_libraries(test_section
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_section)

# Assembler tests
add_executable(test_assembler
    test_assembler.cpp
)

target_link_libraries(test_assembler
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_assembler)

# CPU 6502 tests
add_executable(test_cpu6502
    test_cpu6502.cpp
)

target_link_libraries(test_cpu6502
    xasm_cpu
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_cpu6502)

# SimpleSyntax tests
add_executable(test_simple_syntax
    test_simple_syntax.cpp
)

target_link_libraries(test_simple_syntax
    xasm_syntax
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_simple_syntax)

# MerlinSyntax tests
add_executable(test_merlin_syntax
    test_merlin_syntax.cpp
)

target_link_libraries(test_merlin_syntax
    xasm_syntax
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_merlin_syntax)

# BinaryOutput tests
add_executable(test_binary_output
  test_binary_output.cpp
)

target_link_libraries(test_binary_output
  xasm_output
  xasm_core
  gtest_main
  gtest
)

gtest_discover_tests(test_binary_output)

# CLI tests
add_executable(test_cli
  test_cli.cpp
  ../../src/cli_parser.cpp
)

target_link_libraries(test_cli
  CLI11::CLI11
  gtest_main
  gtest
)

target_include_directories(test_cli PRIVATE
  ${PROJECT_SOURCE_DIR}/include
)

gtest_discover_tests(test_cli)
