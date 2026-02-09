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

# ParseHex tests
add_executable(test_parse_hex
    test_parse_hex.cpp
)

target_link_libraries(test_parse_hex
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_parse_hex)

# CPU 6502 tests
add_executable(test_cpu6502
    test_cpu6502.cpp
    test_65816_extended.cpp
)

target_link_libraries(test_cpu6502
    xasm_cpu
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_cpu6502)

# CPU 6809 tests
add_executable(test_cpu6809
    test_cpu6809.cpp
)

target_link_libraries(test_cpu6809
    xasm_cpu
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_cpu6809)

# CPU 6809 Indexed Addressing tests
add_executable(test_cpu6809_indexed
    test_cpu6809_indexed.cpp
)

target_link_libraries(test_cpu6809_indexed
    xasm_cpu
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_cpu6809_indexed)

# CPU Z80 tests
add_executable(test_cpu_z80
    test_cpu_z80.cpp
)

target_link_libraries(test_cpu_z80
    xasm_cpu
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_cpu_z80)

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

# XC Directive tests (Merlin CPU mode switching)
add_executable(test_xc_directive
    test_xc_directive.cpp
)

target_link_libraries(test_xc_directive
    xasm_syntax
    xasm_cpu
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_xc_directive)

# ScmasmSyntax tests
add_executable(test_scmasm_syntax
    test_scmasm_syntax.cpp
)

target_link_libraries(test_scmasm_syntax
    xasm_syntax
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_scmasm_syntax)

# EdtasmSyntax tests
add_executable(test_edtasm_syntax
    test_edtasm_syntax.cpp
)

target_link_libraries(test_edtasm_syntax
    xasm_syntax
    xasm_cpu
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_edtasm_syntax)

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

# Binary Compatibility tests (vasm parity)
add_executable(test_binary_compat
  test_binary_compat.cpp
)

target_link_libraries(test_binary_compat
  xasm_syntax
  xasm_output
  xasm_core
  gtest_main
  gtest
)

gtest_discover_tests(test_binary_compat)

# ListingOutput tests
add_executable(test_listing_output
  test_listing_output.cpp
)

target_link_libraries(test_listing_output
  xasm_output
  xasm_core
  gtest_main
  gtest
)

gtest_discover_tests(test_listing_output)

# SymbolOutput tests
add_executable(test_symbol_output
  test_symbol_output.cpp
)

target_link_libraries(test_symbol_output
  xasm_output
  xasm_core
  gtest_main
  gtest
)

gtest_discover_tests(test_symbol_output)

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

# Exception safety tests
add_executable(test_exception_safety
  test_exception_safety.cpp
)

target_link_libraries(test_exception_safety
  xasm_core
  xasm_cpu
  gtest_main
  gtest
)

gtest_discover_tests(test_exception_safety)

# Error reporting tests
add_executable(test_error_reporting
  test_error_reporting.cpp
)

target_link_libraries(test_error_reporting
  xasm_syntax
  xasm_core
  gtest_main
  gtest
)

gtest_discover_tests(test_error_reporting)

# Const correctness tests
add_executable(test_const_correctness
  test_const_correctness.cpp
)

target_link_libraries(test_const_correctness
  xasm_core
  xasm_cpu
  gtest_main
  gtest
)

gtest_discover_tests(test_const_correctness)

# Address value object tests (Tier 2.2 refactoring)
add_executable(test_address
    test_address.cpp
)

target_link_libraries(test_address
    gtest_main
    gtest
)

gtest_discover_tests(test_address)

# ErrorFormatter tests
add_executable(test_error_formatter
    test_error_formatter.cpp
)

target_link_libraries(test_error_formatter
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_error_formatter)

# Stoul fix tests (Bug: Expression evaluator stoul conversion)
add_executable(test_stoul_fix
    test_stoul_fix.cpp
)

target_link_libraries(test_stoul_fix
    xasm_syntax
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_stoul_fix)

# FlexAsmSyntax tests
add_executable(test_flex_syntax
    test_flex_syntax.cpp
)

target_link_libraries(test_flex_syntax
    xasm_syntax
    xasm_cpu
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_flex_syntax)

# FlexAsmSyntax Macro tests (Phase 2)
add_executable(test_flex_macros
    test_flex_macros.cpp
)

target_link_libraries(test_flex_macros
    xasm_syntax
    xasm_cpu
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_flex_macros)

# MacroProcessor tests (Phase 2.1)
add_executable(test_macro_processor
    test_macro_processor.cpp
)

target_link_libraries(test_macro_processor
    xasm_syntax
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_macro_processor)

# FlexAsmSyntax Conditional Assembly tests (Phase 3)
add_executable(test_flex_conditionals
    test_flex_conditionals.cpp
)

target_link_libraries(test_flex_conditionals
    xasm_syntax
    xasm_cpu
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_flex_conditionals)

# Z80 Universal Syntax tests
add_executable(test_z80_universal_syntax
    test_z80_universal_syntax.cpp
)

target_link_libraries(test_z80_universal_syntax
    xasm_syntax
    xasm_cpu
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_z80_universal_syntax)

# Intel HEX Writer tests
add_executable(test_intel_hex_writer
    test_intel_hex_writer.cpp
)

target_link_libraries(test_intel_hex_writer
    xasm_output
    xasm_core
    gtest_main
    gtest
)

gtest_discover_tests(test_intel_hex_writer)

